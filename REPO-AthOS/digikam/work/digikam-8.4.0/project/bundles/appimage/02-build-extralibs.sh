#!/bin/bash

# Script to build extra libraries using Linux host.
# This script must be run as sudo
#
# SPDX-FileCopyrightText: 2015-2024 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

#################################################################################################
# Manage script traces to log file

mkdir -p ./logs
exec > >(tee ./logs/build-extralibs.full.log) 2>&1

#################################################################################################

echo "02-build-extralibs.sh : build extra libraries."
echo "----------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./common.sh
. ./config.sh
ChecksRunAsRoot
StartScript
ChecksCPUCores
HostAdjustments
RegisterRemoteServers

ORIG_WD="`pwd`"

#################################################################################################

cd $BUILDING_DIR

rm -rf $BUILDING_DIR/* || true

cmake $ORIG_WD/../3rdparty \
      -DCMAKE_INSTALL_PREFIX:PATH=/usr \
      -DINSTALL_ROOT=/usr \
      -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
      -DEXTERNALS_BUILD_DIR=$BUILDING_DIR \
      -DKA_VERSION=$DK_KA_VERSION \
      -DKP_VERSION=$DK_KP_VERSION \
      -DKDE_VERSION=$DK_KDE_VERSION \
      -DENABLE_QTVERSION=$DK_QTVERSION

# NOTE: The order to compile each component here is very important.

# core KF5 frameworks dependencies
cmake --build . --config RelWithDebInfo --target ext_extra-cmake-modules        -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kconfig                    -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_breeze-icons               -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kcoreaddons                -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kwindowsystem              -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_solid                      -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_threadweaver               -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_karchive                   -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kdbusaddons                -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_ki18n                      -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kcrash                     -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kcodecs                    -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kauth                      -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kguiaddons                 -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kwidgetsaddons             -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kitemviews                 -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kcompletion                -- -j$CPU_CORES

if [[ $DK_QTVERSION == 6 ]] ; then

    cmake --build . --config RelWithDebInfo --target ext_kcolorscheme           -- -j$CPU_CORES

fi

cmake --build . --config RelWithDebInfo --target ext_kconfigwidgets             -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kiconthemes                -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kservice                   -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kglobalaccel               -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kxmlgui                    -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kbookmarks                 -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kimageformats              -- -j$CPU_CORES

# Extra support for digiKam

# Desktop integration support
cmake --build . --config RelWithDebInfo --target ext_knotifications             -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kjobwidgets                -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_kio                        -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_knotifyconfig              -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_sonnet                     -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_ktextwidgets               -- -j$CPU_CORES

# libksane support
if [[ $DK_QTVERSION == 6 ]] ; then

    cmake --build . --config RelWithDebInfo --target ext_qca                    -- -j$CPU_CORES
    cmake --build . --config RelWithDebInfo --target ext_kwallet                -- -j$CPU_CORES
    cmake --build . --config RelWithDebInfo --target ext_ksanecore              -- -j$CPU_CORES

fi

cmake --build . --config RelWithDebInfo --target ext_libksane                   -- -j$CPU_CORES

# Calendar support
cmake --build . --config RelWithDebInfo --target ext_kcalendarcore              -- -j$CPU_CORES

# Platform Input Context Qt plugin
cmake --build . --config RelWithDebInfo --target ext_fcitx-qt                   -- -j$CPU_CORES

# Breeze style support
cmake --build . --config RelWithDebInfo --target ext_breeze                     -- -j$CPU_CORES

#################################################################################################

if [[ $DK_QTVERSION == 6 ]] ; then

    KF6_GITREV_LST=$ORIG_WD/data/kf6_manifest.txt

    echo "List git sub-module revisions in $KF6_GITREV_LST"

    if [ -f $KF6_GITREV_LST ] ; then
        rm -f $KF6_GITREV_LST
    fi

    currentDate=`date +"%Y-%m-%d"`
    echo "+KF6 Snapshot $currentDate" > $KF6_GITREV_LST

    # --- List git revisions for all sub-modules

    DIRS=$(find $BUILDING_DIR/ext_kf6/ -name "ext_*-prefix")

    for ITEM in $DIRS ; do

        COMPONENT=$(echo $ITEM | cut -d'_' -f 1 | cut -d'-' -f 2)
        SUBDIR=$BUILDING_DIR/ext_kf6/$ITEM/src/$COMPONENT
        cd $SUBDIR
        echo "$(basename "$SUBDIR"):$(git rev-parse HEAD)" >> $KF6_GITREV_LST
        cd $ORIG_WD

    done

    cat $KF6_GITREV_LST

fi

TerminateScript
