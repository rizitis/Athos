#!/bin/bash

grep -hEo '^SRCURL=[^ ]+' ../REPO-AthOS/*/*.ini | cut -d= -f2 | sort > urls.txt

grep -hEo '^SRCURL=[^ ]+' ../REPO-AthOS/*/*.ini | cut -d= -f2 | sort > urls_by_name.txt
