#!/bin/bash

echo "Looking for kde-stable"
python kde_stable_checker.py 

wait
echo ""
cat kde_update_output.txt
echo ""

python plasma_stable_checker.py

wait
echo ""
cat plasma_update_output.txt
echo ""

echo "Looking git{hub,lab}"
python GIT_FINDER.py

echo "Github"
echo "==="
cat need-update-github.txt
echo ""

python NEW_RELEASE_FINDER.py

echo "Gitlab"
echo "==="
cat need-update-gitlab.txt
