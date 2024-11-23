#!/bin/bash

# SPDX-FileCopyrightText: 2013-2024 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Run CppCheck static analyzer on whole digiKam source code.
# http://cppcheck.sourceforge.net/
# Dependencies : Python::pygments module to export report as HTML.
#
# If '--nowebupdate' is passed as argument, static analyzer results are not pushed online at
# https://files.kde.org/digikam/reports/ (default yes).
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

. ./common.sh

# Analyzer configuration.
. ../../.cppcheck

checksCPUCores

ORIG_WD="`pwd`"
REPORT_DIR="${ORIG_WD}/report.cppcheck"
WEBSITE_DIR="${ORIG_WD}/site"

# Get active git branches to create report description string
TITLE="digiKam-$(parseGitBranch)$(parseGitHash)"
echo "CppCheck Static Analyzer task name: $TITLE"

rm -fr $REPORT_DIR
rm -fr $WEBSITE_DIR

# Print the skipped directories taken from the config file.

for DROP_ITEM in $IGNORE_DIRS ; do

    if [[ $DROP_ITEM != *-i ]] ; then

        echo "Skipped dir: $DROP_ITEM"

    fi

done

echo "Cppcheck defines     : $CPPCHECK_DEFINES"
echo "Cppcheck options     : $CPPCHECK_OPTIONS"
echo "Cppcheck suppressions: $CPPCHECK_SUPPRESSIONS"

# List sub-dirs with headers to append as cppcheck includes paths
HDIRS=$(find ../../core -name '*.h' -printf '%h\n' | sort -u)

for INCLUDE_PATH in $HDIRS ; do
    INCLUDE_DIRS+="-I $INCLUDE_PATH/ "
done

cppcheck -j$CPU_CORES \
         $CPPCHECK_DEFINES \
         --verbose \
         $CPPCHECK_OPTIONS \
         --enable=all \
         --check-level=exhaustive \
         $CPPCHECK_SUPPRESSIONS \
         --xml-version=2 \
         --output-file=report.cppcheck.xml \
         $IGNORE_DIRS \
         $INCLUDE_DIRS \
         ../../core

cppcheck-htmlreport --file=report.cppcheck.xml \
                    --report-dir=$REPORT_DIR \
                    --source-dir=. \
                    --title=$TITLE

if [[ $1 != "--nowebupdate" ]] ; then

    updateOnlineReport "cppcheck" $REPORT_DIR $TITLE $(parseGitBranch)

fi

cd $ORIG_DIR

