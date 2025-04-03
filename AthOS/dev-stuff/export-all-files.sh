#!/bin/bash

# Path to the directory
directory="../REPO-AthOS/"

# Collect folder names
folders=$(ls -d "$directory"*/ 2>/dev/null | xargs -n 1 basename)

# Print folder names in a single line
if [ -z "$folders" ]; then
    echo "No folders found in $directory"
else
DATE=$(date)
    echo "# $DATE" > ../../BUILDS
    echo "" >> ../../BUILDS
    echo "$folders" >> ../../BUILDS
fi
