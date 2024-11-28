#!/bin/bash

# Directory to search
directory="../../REPO-AthOS/"

# Remove old output files
rm -f files_with_version_*.txt || true

# Temporary file to store version mapping
temp_file=$(mktemp)

# Find all .ini files and extract their versions
find "$directory" -type f -name "*.ini" | while read -r ini_file; do
    # Extract VERSION= value
    version=$(grep -E '^VERSION=' "$ini_file" | cut -d'=' -f2 | tr -d '[:space:]')

    if [ -n "$version" ]; then
        # Get the file's base name without .ini extension
        file_base=$(basename "$ini_file" .ini)
        # Save to temporary mapping file
        echo "$version $file_base" >> "$temp_file"
    fi
done

# Process the temporary file to group files by version
sort "$temp_file" | awk '
{
    version = $1; file = $2
    version_files[version] = (version_files[version] ? version_files[version] " " : "") file
    version_counts[version]++
}
END {
    for (version in version_files) {
        if (version_counts[version] > 1) {
            output_file = "files_with_version_" version ".txt"
            print version_files[version] > output_file
            print "Created: " output_file
        }
    }
}
'

# Clean up
rm -f "$temp_file"
