#!/usr/bin/env python3
import os
import re
import requests
from packaging.version import Version, InvalidVersion

def extract_srcurl(file_path):
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            for line in f:
                if line.startswith("SRCURL="):
                    return line.strip().split("=", 1)[1]
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
    return None

def get_latest_github_release(repo_url):
    match = re.search(r'github\.com/([^/]+/[^/]+)', repo_url)
    if not match:
        return None
    repo = match.group(1)
    api_url = f"https://api.github.com/repos/{repo}/releases/latest"
    try:
        response = requests.get(api_url, timeout=5)
        response.raise_for_status()
        return response.json().get("tag_name", "")
    except (requests.RequestException, ValueError):
        return None

def compare_versions(current_version, latest_version):
    try:
        if not isinstance(latest_version, str) or not isinstance(current_version, str):
            return False
        return Version(latest_version) > Version(current_version)
    except InvalidVersion:
        return False

def main(directory):
    need_update = []
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(".ini"):
                file_path = os.path.join(root, file)
                src_url = extract_srcurl(file_path)
                if src_url and "github.com" in src_url:
                    latest_version = get_latest_github_release(src_url)
                    if latest_version:
                        current_version_match = re.search(r'\d+(?:\.\d+)+', src_url)
                        if current_version_match:
                            current_version = current_version_match.group()
                            if compare_versions(current_version, latest_version):
                                need_update.append(file)

    with open("need-update2.txt", "w", encoding="utf-8") as f:
        for file in need_update:
            f.write(file + "\n")
    print("Check completed. See need-update2.txt for outdated files.")

if __name__ == "__main__":
    main("../REPO-AthOS")
