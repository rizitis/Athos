#!/bin/bash

# Target directory for autostart
TARGET_DIR="/etc/kde/xdg/autostart"

# List of .desktop filenames to find and copy
FILES_TO_FIND=(
    "gmenudbusmenuproxy.desktop"
    "kaccess.desktop"
    "kalarm.autostart.desktop"
    "kglobalacceld.desktop"
    "kmix_autostart.desktop"
    "konqy_preload.desktop"
    "org.kde.kalendarac.desktop"
    "org.kde.kdeconnect.daemon.desktop"
    "org.kde.kgpg.desktop"
    "org.kde.plasma-fallback-session-restore.desktop"
    "org.kde.plasmashell.desktop"
    "org.kde.xwaylandvideobridge.desktop"
    "pam_kwallet_init.desktop"
    "polkit-kde-authentication-agent-1.desktop"
    "powerdevil.desktop"
    "restore_kmix_volumes.desktop"
    "xembedsniproxy.desktop"
)

# Ensure the target directory exists
if [ ! -d "$TARGET_DIR" ]; then
    echo "Target directory does not exist: $TARGET_DIR"
    echo "Creating it now..."
    mkdir -p "$TARGET_DIR" || { echo "Failed to create $TARGET_DIR"; exit 1; }
fi

# Scan the system and copy matching files
echo "Scanning the system for specified .desktop files..."

for file in "${FILES_TO_FIND[@]}"; do
    # Find the file on the system
    found_file=$(find / -type f -name "$file" 2>/dev/null | head -n 1)
    
    if [ -n "$found_file" ]; then
        echo "Found: $found_file"
        cp "$found_file" "$TARGET_DIR"
    else
        echo "Not found: $file"
    fi
done

# Verify the result
echo "The following files are now in $TARGET_DIR:"
ls -l "$TARGET_DIR"

echo "Done!"
