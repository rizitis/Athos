#!/bin/bash
CWD=$(pwd)

if [ -e "$CWD/REBUILD.list" ]; then
    while IFS= read -r line; do
        athos "$line"
    done < "$CWD"/REBUILD.list
else
    echo "File upgrade-plasma5.list not found."
fi
