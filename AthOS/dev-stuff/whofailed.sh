#!/bin/bash
cd ../REPO-AthOS || exit
date
echo "Following packages failed to build:"
find ./ -type d -name work -exec dirname {} \;
