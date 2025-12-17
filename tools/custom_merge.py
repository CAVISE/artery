#!/usr/bin/env python3
import asyncio
import argparse
import json
import shutil
import sys
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
    except Exception:
        return {}

def is_binary(filepath: Path) -> bool:
    '''Check if a file is binary by searching for a null byte in the first 8KB.'''
    try:
        with open(filepath, 'rb') as f:
            return b'\x00' in f.read(8192)
    except Exception:
        return False

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
    try:
        proc = await asyncio.create_subprocess_exec(
            'git', 'rev-parse', '-q', '--verify', ref,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.DEVNULL
        )
        stdout, _ = await proc.communicate()
        return stdout.decode().strip()
    except Exception:
        return ''

async def is_upstream_merge(config: Config) -> bool:
    '''Check if the current operation is a merge from the upstream branch.'''
    remote = config.get('upstream_remote', 'upstream')
    branch = config.get('upstream_branch', 'master')
    target_ref = f'{remote}/{branch}'

    merge_head = await get_git_rev('MERGE_HEAD')
    upstream_rev = await get_git_rev(target_ref)

    return merge_head == upstream_rev if merge_head else False

async def main() -> None:
    parser = argparse.ArgumentParser(description='Custom Git Merge Driver')
    parser.add_argument('base', type=Path, help='%O: Ancestor file')
    parser.add_argument('current', type=Path, help='%A: Current branch file')
    parser.add_argument('other', type=Path, help='%B: Other branch file')
    parser.add_argument('path', type=str, help='%P: Original file path')
    args = parser.parse_args()

    script_dir = Path(__file__).parent.resolve()
    config = load_config(script_dir / 'merge_config.json')
    norm_path = Path(args.path).as_posix()

    # 1. Binary check
    if is_binary(args.current) or is_binary(args.other):
        print(f'[Custom Merge] Binary: {norm_path} -> Strategy: KEEP OURS')
        sys.exit(0)

    # 2. UPSTREAM CHECK
    if not await is_upstream_merge(config):
        sys.exit(await run_git_merge_file(args.base, args.current, args.other))

    # 3. Path-specific rules
    for path_prefix in config.get('prefer_theirs', []):
        if norm_path.startswith(path_prefix):
            print(f'[Custom Merge] Path rule: {norm_path} -> Strategy: PREFER THEIRS')
            shutil.copyfile(args.other, args.current)
            sys.exit(0)

    for path_prefix in config.get('prefer_ours', []):
        if norm_path.startswith(path_prefix):
            print(f'[Custom Merge] Path rule: {norm_path} -> Strategy: PREFER OURS')
            sys.exit(0)

    # 4. Default behavior for upstream
    behavior = config.get('default_behavior', 'manual_resolution')
    if behavior == 'prefer_theirs':
        shutil.copyfile(args.other, args.current)
        sys.exit(0)
    elif behavior == 'prefer_ours':
        sys.exit(0)
    else:
        sys.exit(await run_git_merge_file(args.base, args.current, args.other))

if __name__ == '__main__':
    asyncio.run(main())
