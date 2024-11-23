#!/bin/bash

# Script to update Linux AppImage bundles.
#
# SPDX-FileCopyrightText: 2015-2024 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

########################################################################
# Function to upload host log files
BundleUploadLogFiles()
{

if [[ $DK_UPLOAD = 1 ]] ; then

    if [[ $DK_QTVERSION == 5 ]] ; then

        QT_SUF="-Qt5"

    else

        QT_SUF="-Qt6"

    fi

    echo -e "---------- Cleanup older host logs from files.kde.org repository \n"

    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR/build.logs/lin64$QT_SUF <<< "rm build-host.full.log.gz"
    sftp -q $DK_UPLOADURL:$DK_UPLOADDIR/build.logs/lin64$QT_SUF <<< "rm build-extralibs.full.log.gz"

    echo -e "---------- Compress host log files \n"

    gzip -k $ORIG_WD/logs/build-host.full.log $ORIG_WD/logs/build-host.full.log.gz           || true
    gzip -k $ORIG_WD/logs/build-extralibs.full.log $ORIG_WD/logs/build-extralibs.full.log.gz || true

    echo -e "---------- Upload new host logs to files.kde.org repository \n"

    rsync -r -v --progress -e ssh $ORIG_WD/logs/build-host.full.log.gz $DK_UPLOADURL:$DK_UPLOADDIR/build.logs/lin64$QT_SUF      || true
    rsync -r -v --progress -e ssh $ORIG_WD/logs/build-extralibs.full.log.gz $DK_UPLOADURL:$DK_UPLOADDIR/build.logs/lin64$QT_SUF || true

    echo -e "---------- Cleanup local bundle log file archives \n"

    rm -f $ORIG_WD/logs/build-host.full.log.gz      || true
    rm -f $ORIG_WD/logs/build-extralibs.full.log.gz || true

fi

}

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR
trap BundleUploadLogFiles ERR exit

ORIG_WD="`pwd`"

#################################################################################################
# Pre-processing checks

. ./common.sh
. ./config.sh
ChecksRunAsRoot
ChecksCPUCores
HostAdjustments

# ---

StartScript

echo "+++++++++++++++++++++++ Update Linux AppImage bundle ++++++++++++++++++++++++++++++"
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

./03-build-digikam.sh

sed -e "s/DK_DEBUG=1/DK_DEBUG=0/g"           ./config.sh > ./tmp.sh ; mv -f ./tmp.sh ./config.sh

./04-build-appimage.sh

sed -e "s/DK_DEBUG=0/DK_DEBUG=1/g"           ./config.sh > ./tmp.sh ; mv -f ./tmp.sh ./config.sh

./04-build-appimage.sh

sed -e "s/DK_DEBUG=1/DK_DEBUG=0/g"           ./config.sh > ./tmp.sh ; mv -f ./tmp.sh ./config.sh

echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

TerminateScript
