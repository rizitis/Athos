#!/bin/bash
CWD=$(pwd)

if [ -e ../"$CWD/upgrade-plasma5.list" ]; then
    while IFS= read -r line; do
        ls /var/adm/packages | grep "$line"
    done < ../"$CWD"/upgrade-plasma5.list
else
    echo "File upgrade-plasma5.list not found."
fi
