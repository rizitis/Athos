#!/bin/bash
APPL=AthOS

. /etc/"$APPL"/athos.env || exit

for var in $(set | awk -F= '/^VAR_/ {print $1}'); do
    dir="${!var}"
    if [ -e "$dir" ]; then
        rm -r "$dir" || exit
        echo "Deleted: $dir"
    else
        echo "Not found: $dir"
    fi
done


