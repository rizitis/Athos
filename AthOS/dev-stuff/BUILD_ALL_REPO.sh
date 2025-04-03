#!/bin/bash
CWD=$(pwd)

if [ -e "$CWD/build-all.txt" ]; then
    while IFS= read -r line; do
        athos "$line"
    done < "$CWD"/build-all.txt
else
    echo "File build-all.txt not found."
fi
