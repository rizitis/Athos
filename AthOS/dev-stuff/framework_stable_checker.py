import hashlib
import requests
import os

URL = "https://download.kde.org/stable/frameworks/?C=M;O=D"
CACHE_FILE = os.path.expanduser("framework_content_hash.txt")
OUTPUT_FILE = os.path.expanduser("framework_update_output.txt")

# Fetch the webpage content
response = requests.get(URL)
if response.status_code != 200:
    print("Failed to fetch the webpage.")
    exit(1)

# Generate a hash of the page content
current_hash = hashlib.sha256(response.content).hexdigest()

# If the cache file doesn't exist, create it and store the hash
if not os.path.exists(CACHE_FILE):
    with open(CACHE_FILE, "w") as f:
        f.write(current_hash)
    with open(OUTPUT_FILE, "w") as f:
        f.write("First run. Saved the page content hash.\n")
    exit(0)

# Read the previous hash from the cache file
with open(CACHE_FILE, "r") as f:
    previous_hash = f.read().strip()

# Compare the current and previous hashes
if current_hash != previous_hash:
    with open(OUTPUT_FILE, "w") as f:
        f.write("The FRAMEWORK page has changed!\n")
    with open(CACHE_FILE, "w") as f:
        f.write(current_hash)  # Update the stored hash
else:
    with open(OUTPUT_FILE, "w") as f:
        f.write("No changes detected on the FRAMEWORK page.\n")
