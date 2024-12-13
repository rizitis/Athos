#!/bin/bash

# Get the current working directory
CWD=$(pwd)

# Source environment variables
. NEW-VERSIONS.env
. CURRENT-VERSIONS.env

# Define the log file
LOG_FILE="file_list.log"

# Sanity check for CWD
if [ ! -d "$CWD" ]; then
  echo "Error: Current working directory ($CWD) is invalid."
  exit 1
fi

# Generate the file list log
echo "Scanning for .ini files in $CWD..."
find $CWD -type f -name "*.ini" -not -path "*/.*" > "$LOG_FILE"

# Inform the user
echo "The following .ini files will be processed with these simulated changes:"
echo > "$LOG_FILE"  # Clear the log file
while read -r file; do
  echo "File: $file" >> "$LOG_FILE"
  echo "----" >> "$LOG_FILE"
  sed "s/$OLD_FRAMEv/$NEW_FRAMEv/g" "$file" >> "$LOG_FILE"
  wait
  sed "s/$OLD_FRAME/$NEW_FRAME/g" "$file" >> "$LOG_FILE"
  sed "s/$OLD_PLASMA/$NEW_PLASMA/g" "$file" >> "$LOG_FILE"
  sed "s/$OLD_APPS/$NEW_APPS/g" "$file" >> "$LOG_FILE"
  echo >> "$LOG_FILE"
done < <(find "$CWD" -type f -name "*.ini" -not -path "*/.*")

# Display summary
echo "Simulated output saved to $LOG_FILE."
echo
echo "Review the changes in $LOG_FILE."
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
  done < <(find "$CWD" -type f -name "*.ini" -not -path "*/.*")

  echo "Update completed."
else
  echo "Operation canceled."
fi
