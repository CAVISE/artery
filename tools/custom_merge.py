#!/usr/bin/env python3
import asyncio
import argparse
import json
import shutil
import sys
import subprocess
from pathlib import Path
from typing import TypedDict, List

class Config(TypedDict, total=False):
    upstream_remote: str
    upstream_branch: str
    default_behavior: str
    prefer_theirs: List[str]
    prefer_ours: List[str]

def load_config(config_path: Path) -> Config:
    '''Load merge configuration from JSON file.'''
    if not config_path.exists():
        return {}
    try:
        return json.loads(config_path.read_text(encoding='utf-8'))
    except json.JSONDecodeError:
        # Catching only specific JSON error as requested
        return {}

async def run_git_merge_file(base: Path, current: Path, other: Path) -> int:
    '''Run the standard git merge-file command asynchronously.'''
    cmd = [
        'git', 'merge-file', 
        '-L', 'current', '-L', 'base', '-L', 'incoming',
        str(current), str(base), str(other)
    ]
    proc = await asyncio.create_subprocess_exec(*cmd)
    return await proc.wait()

async def get_git_rev(ref: str) -> str:
    '''Resolve a git reference to a commit hash asynchronously.'''
    proc = await asyncio.create_subprocess_exec(
        'git', 'rev-parse', '-q', '--verify', ref,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.DEVNULL
    )
    stdout, _ = await proc.communicate()
    if proc.returncode != 0:
        return ''
    return stdout.decode().strip()

async def is_upstream_merge(config: Config) -> bool:
    '''Check if the current operation is a merge from the upstream branch.'''
    remote = config.get('upstream_remote', 'upstream')
    branch = config.get('upstream_branch', 'master')
    target_ref = f'{remote}/{branch}'

    merge_head = await get_git_rev('MERGE_HEAD')
    upstream_rev = await get_git_rev(target_ref)

    return merge_head == upstream_rev if merge_head else False

async def resolve_structural_conflicts(config: Config):
    '''
    Finds structural conflicts (like add/add, status 'AA') and resolves them 
    using the logic from the JSON configuration (Asynchronous version).
    '''
    print("[Custom Merge] Scanning for structural conflicts (add/add)...")
    
    proc = await asyncio.create_subprocess_exec(
        'git', 'status', '--porcelain',
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE
    )
    stdout, _ = await proc.communicate()
    
    if proc.returncode != 0:
        return

    status_output = stdout.decode()

    for line in status_output.splitlines():
        if line.startswith('AA '):
            file_path = line[3:].strip()
            norm_path = Path(file_path).as_posix()
            
            strategy = None
            for path_prefix in config.get('prefer_theirs', []):
                if norm_path.startswith(path_prefix):
                    strategy = 'theirs'
                    break
            
            if not strategy:
                for path_prefix in config.get('prefer_ours', []):
                    if norm_path.startswith(path_prefix):
                        strategy = 'ours'
                        break
            
            if not strategy:
                behavior = config.get('default_behavior', 'manual_resolution')
                if behavior == 'prefer_theirs':
                    strategy = 'theirs'
                elif behavior == 'prefer_ours':
                    strategy = 'ours'

            if strategy:
                print(f'[Custom Merge] AA Conflict: {norm_path} -> Strategy: PREFER {strategy.upper()}')
                c_proc = await asyncio.create_subprocess_exec('git', 'checkout', f'--{strategy}', file_path)
                await c_proc.wait()
                a_proc = await asyncio.create_subprocess_exec('git', 'add', file_path)
                await a_proc.wait()
            else:
                print(f'[Custom Merge] AA Conflict: {norm_path} -> Left for MANUAL resolution')

async def main() -> int:
    usage_info = """
Setup instructions:
  git config merge.custom-driver.name "Custom merge driver for Artery/Cavise"
  git config merge.custom-driver.driver "python3 tools/custom_merge.py %O %A %B %P"
    """

    parser = argparse.ArgumentParser(
        description='Custom Git Merge Driver',
        epilog=usage_info,
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    # Mode flag for structural conflicts
    parser.add_argument('--fix-structural', action='store_true', help='Handle add/add conflicts after merge')
    
    # Git driver arguments (positional)
    parser.add_argument('base', type=Path, nargs='?', help='O: Ancestor file')
    parser.add_argument('current', type=Path, nargs='?', help='A: Current branch file')
    parser.add_argument('other', type=Path, nargs='?', help='B: Other branch file')
    parser.add_argument('path', type=str, nargs='?', help='P: Original file path')
    
    args = parser.parse_args()

    script_dir = Path(__file__).parent.resolve()
    config = load_config(script_dir / 'merge_config.json')

    # --- MODE: Structural Fix ---
    if args.fix_structural:
        await resolve_structural_conflicts(config)
        return 0

    # --- MODE: Content Merge (Standard Driver) ---
    if not all([args.base, args.current, args.other, args.path]):
        parser.print_help()
        return 1

    norm_path = Path(args.path).as_posix()

    # 1. Binary check
    if is_binary(args.current) or is_binary(args.other):
        print(f'[Custom Merge] Binary: {norm_path} -> Strategy: KEEP OURS')
        return 0

    # 2. UPSTREAM CHECK
    if not await is_upstream_merge(config):
        return await run_git_merge_file(args.base, args.current, args.other)

    # 3. Path-specific rules
    for path_prefix in config.get('prefer_theirs', []):
        if norm_path.startswith(path_prefix):
            print(f'[Custom Merge] Path rule: {norm_path} -> Strategy: PREFER THEIRS')
            shutil.copyfile(args.other, args.current)
            return 0

    for path_prefix in config.get('prefer_ours', []):
        if norm_path.startswith(path_prefix):
            print(f'[Custom Merge] Path rule: {norm_path} -> Strategy: PREFER OURS')
            return 0

    # 4. Default behavior for upstream
    behavior = config.get('default_behavior', 'manual_resolution')
    if behavior == 'prefer_theirs':
        shutil.copyfile(args.other, args.current)
        return 0
    elif behavior == 'prefer_ours':
        return 0
    else:
        return await run_git_merge_file(args.base, args.current, args.other)

def is_binary(filepath: Path) -> bool:
    '''Check if a file is binary by searching for a null byte in the first 8KB.'''
    try:
        with open(filepath, 'rb') as f:
            return b'\x00' in f.read(8192)
    except OSError:
        return False

if __name__ == '__main__':
    sys.exit(asyncio.run(main()))
