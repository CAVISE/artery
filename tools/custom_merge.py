#!/usr/bin/env python3
import sys
import os
import shutil
import subprocess
import json

# Аргументы, которые Git передает драйверу (настраиваются в .git/config)
# %O = предок (base), %A = наша версия (current), %B = их версия (other), %P = путь к файлу
try:
    base_file = sys.argv[1]
    current_file = sys.argv[2]
    other_file = sys.argv[3]
    file_path = sys.argv[4]
except IndexError:
    print("Error: Not enough arguments provided to merge driver.")
    sys.exit(1)

# Путь к конфигу (лежит рядом со скриптом)
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
CONFIG_PATH = os.path.join(SCRIPT_DIR, "merge_config.json")

def load_config():
    if not os.path.exists(CONFIG_PATH):
        return {}
    with open(CONFIG_PATH, 'r') as f:
        return json.load(f)

def run_git_default_merge():
    """
    Запускает стандартный git merge-file для создания маркеров конфликта.
    Возвращает код возврата git merge-file (0 если без конфликтов, >0 если конфликт).
    """
    cmd = ["git", "merge-file", "-L", "current", "-L", "base", "-L", "incoming", 
           current_file, base_file, other_file]
    return subprocess.call(cmd)

def is_upstream_merge(config):
    """
    Пытается определить, мержим ли мы upstream/master.
    Проверяем MERGE_HEAD и сравниваем его с хешом апстрим ветки.
    """
    upstream_remote = config.get("upstream_remote", "upstream")
    upstream_branch = config.get("upstream_branch", "master")
    target_ref = f"{upstream_remote}/{upstream_branch}"

    # Получаем хеш коммита, который мержится (MERGE_HEAD)
    try:
        merge_head = subprocess.check_output(["git", "rev-parse", "-q", "--verify", "MERGE_HEAD"], text=True).strip()
        
        # Получаем хеш upstream/master
        upstream_rev = subprocess.check_output(["git", "rev-parse", "-q", "--verify", target_ref], text=True).strip()
        
        if not merge_head:
            # Если MERGE_HEAD нет, возможно это cherry-pick или rebase, лучше отдать гиту
            return False
            
        return merge_head == upstream_rev
    except subprocess.CalledProcessError:
        return False

def is_binary_file(file_path):
    """Проверяет, является ли файл бинарным, используя git diff."""
    try:
         result = subprocess.run(
            ["git", "diff", "--no-index", current_file, other_file],
            capture_output=True,
            text=True,
            check=False
        )
        return "Binary files" in result.stdout or "Binary files" in result.stderr
    except FileNotFoundError:
         return False

def main():
    config = load_config()

    if not is_upstream_merge(config):
        sys.exit(run_git_default_merge())

    if is_binary_file(file_path):
        print(f"[Custom Merge] File: {file_path} -> Strategy: BINARY (Prefer Ours)")
        sys.exit(0)

    behavior = config.get("default_behavior", "manual_resolution")
    
    norm_file_path = file_path.replace("\\", "/")

    # Проверяем prefer_theirs (берем версию Апстрима)
    for path in config.get("prefer_theirs", []):
        if norm_file_path.startswith(path):
            behavior = "prefer_theirs"
            break
            
    # Проверяем prefer_ours (берем нашу версию)
    for path in config.get("prefer_ours", []):
        if norm_file_path.startswith(path):
            behavior = "prefer_ours"
            break

    # 4. Применение стратегии
    print(f"[Custom Merge] File: {file_path} -> Strategy: {behavior}")

    if behavior == "prefer_ours":
        sys.exit(0)
        
    elif behavior == "prefer_theirs":
        shutil.copyfile(other_file, current_file)
        sys.exit(0)
        
    else:
        # manual_resolution: вызываем стандартный тул гита
        sys.exit(run_git_default_merge())

if __name__ == "__main__":
    main()
