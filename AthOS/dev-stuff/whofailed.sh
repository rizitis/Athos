#!/bin/bash
cd ../REPO-AthOS || exit
date
echo "following pacckages failed to build:"
find ./ -type d -name work -exec dirname {} \;
