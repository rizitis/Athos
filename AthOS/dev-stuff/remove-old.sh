#!/bin/bash
# remove old no needed packages from plasma5
CWD=$(pwd)

if [ -e "$CWD/OLD_PKGS.list" ]; then
    while IFS= read -r line; do
        removepkg "$line"
    done < "$CWD"/OLD_PKGS.list
else
    echo "pkg not found."
fi
