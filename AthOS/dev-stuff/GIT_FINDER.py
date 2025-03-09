#!/usr/bin/env python3
import os
import re
import requests
from packaging.version import Version, InvalidVersion

def extract_srcurl(file_path):
    """Extract SRCURL from the given file."""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            for line in f:
                if line.startswith("SRCURL="):
                    return line.strip().split("=", 1)[1]
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
    return None

def get_latest_github_version(repo_url):
    """Get the latest release or tag from a GitHub repository."""
    match = re.search(r'github\.com/([^/]+/[^/]+)', repo_url)
    if not match:
        return None
    repo = match.group(1)

    # Try to get the latest release first
    api_release_url = f"https://api.github.com/repos/{repo}/releases/latest"
    try:
        response = requests.get(api_release_url, timeout=5)
        response.raise_for_status()
        latest_version = response.json().get("tag_name", "")
        if latest_version:
            return latest_version
    except requests.RequestException:
        pass  # Fall back to tags if no releases exist

    # Try to get the latest tag
    api_tags_url = f"https://api.github.com/repos/{repo}/tags"
    try:
        response = requests.get(api_tags_url, timeout=5)
        response.raise_for_status()
        tags = response.json()
        if tags:
            return tags[0]["name"]  # First tag is usually the latest
    except requests.RequestException:
        return None

    return None

def get_latest_gitlab_version(repo_url):
    """Get the latest tag from a GitLab repository."""
    match = re.search(r'gitlab\.com/([^/]+/[^/]+)', repo_url)
    if not match:
        return None
    repo = match.group(1).replace("/", "%2F")  # GitLab API requires encoded slashes
    api_tags_url = f"https://gitlab.com/api/v4/projects/{repo}/repository/tags"

    try:
        response = requests.get(api_tags_url, timeout=5)
        response.raise_for_status()
        tags = response.json()
        if tags:
            return tags[0]["name"]  # First tag is usually the latest
    except requests.RequestException:
        return None

    return None

def compare_versions(current_version, latest_version):
    """Compare version numbers, returning True if an update is needed."""
    try:
        if not isinstance(latest_version, str) or not isinstance(current_version, str):
            return False
        return Version(latest_version) > Version(current_version)
    except InvalidVersion:
        return False

def main(directory):
    need_update_github = []
    need_update_gitlab = []

    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(".ini"):
                file_path = os.path.join(root, file)
                src_url = extract_srcurl(file_path)

                if src_url:
                    latest_version = None
                    if "github.com" in src_url:
                        latest_version = get_latest_github_version(src_url)
                    elif "gitlab.com" in src_url:
                        latest_version = get_latest_gitlab_version(src_url)

                    if latest_version:
                        current_version_match = re.search(r'\d+(?:\.\d+)+', src_url)
                        if current_version_match:
                            current_version = current_version_match.group()
                            if compare_versions(current_version, latest_version):
                                if "github.com" in src_url:
                                    need_update_github.append(file)
                                elif "gitlab.com" in src_url:
                                    need_update_gitlab.append(file)

    with open("need-update-github.txt", "w", encoding="utf-8") as f:
        for file in need_update_github:
            f.write(file + "\n")

    with open("need-update-gitlab.txt", "w", encoding="utf-8") as f:
        for file in need_update_gitlab:
            f.write(file + "\n")

    print("Check completed. See need-update-github.txt and need-update-gitlab.txt for outdated files.")

if __name__ == "__main__":
    main("../REPO-AthOS")
