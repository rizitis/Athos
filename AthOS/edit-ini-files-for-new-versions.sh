#!/bin/bash

# Get the current working directory
CWD=$(pwd)

# Source environment variables
. dev-stuff/NEW-VERSIONS.env
. dev-stuff/CURRENT-VERSIONS.env

# Define the log file
LOG_FILE="file_list.log"

# Sanity check for CWD
if [ ! -d "$CWD" ]; then
  echo "Error: Current working directory ($CWD) is invalid."
  exit 1
fi

# Generate the file list log
echo "Scanning for .ini files in $CWD..."
find "$CWD" -type f -name "*.ini" -not -path "*/.*" > "$LOG_FILE"

# Inform the user
echo "The following .ini files will be processed:"
cat "$LOG_FILE"

echo "File list saved to $LOG_FILE."
echo
echo "If you want to proceed with editing these files, rerun the script with the --edit option."
echo "Example: $0 --edit"
echo
if [ "$1" != "--edit" ]; then
  exit 0
fi

# Edit mode
echo "You are about to modify the listed files. Are you sure? [y/N]"
read -r CONFIRM

if [[ "$CONFIRM" =~ ^[Yy]$ ]]; then
  # Apply edits with sed
  while read -r file; do
    echo "Processing $file"
    sed -i "s/$OLD_FRAMEv/$NEW_FRAMEv/g" "$file"
    sed -i "s/$OLD_FRAME/$NEW_FRAME/g" "$file"
    sed -i "s/$OLD_PLASMA/$NEW_PLASMA/g" "$file"
    sed -i "s/$OLD_APPS/$NEW_APPS/g" "$file"
  done < "$LOG_FILE"

  echo "Update completed."
else
  echo "Operation canceled."
fi

sed -i 's/\/home\/omen\/GITHUB\///g' "$LOG_FILE" || exit
