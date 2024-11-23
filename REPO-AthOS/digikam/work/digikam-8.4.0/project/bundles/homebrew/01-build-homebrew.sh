#! /bin/bash

# Script to build a bundle HomeBrew installation with all digiKam dependencies in a dedicated directory
# This script must be run as sudo
#
# SPDX-FileCopyrightText: 2015      by Shanti <listaccount at revenant dot org>
# SPDX-FileCopyrightText: 2015-2024 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Ask to run as root
#(( EUID != 0 )) && exec sudo -- "$0" "$@"

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

#################################################################################################
# Manage script traces to log file

mkdir -p ./logs
exec > >(tee ./logs/build-homebrew.full.log) 2>&1

#################################################################################################

echo "01-build-homebrew.sh : build a bundle HomeBrew install with digiKam dependencies."
echo "---------------------------------------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./config.sh
. ./common.sh
StartScript
#ChecksRunAsRoot
ChecksXCodeCLI
ChecksCPUCores
OsxCodeName
#RegisterRemoteServers

#################################################################################################

# Paths rules
ORIG_PATH="$PATH"
ORIG_WD="`pwd`"

export PATH=$INSTALL_PREFIX/bin:/$INSTALL_PREFIX/sbin:/$INSTALL_PREFIX/opt/qt6/bin:$ORIG_PATH

#################################################################################################
# Check if a previous bundle already exist

CONTINUE_INSTALL=0

if [ -d "$INSTALL_PREFIX" ] ; then

    read -p "$INSTALL_PREFIX already exist. Do you want to remove it or to continue an aborted previous installation ? [(r)emove/(c)ontinue/(s)top] " answer

    if echo "$answer" | grep -iq "^r" ;then

        echo "---------- Removing existing $INSTALL_PREFIX"
        mv $INSTALL_PREFIX $INSTALL_PREFIX.old || true
        rm -rf $INSTALL_PREFIX.old || true

    elif echo "$answer" | grep -iq "^c" ;then

        echo "---------- Continue aborted previous installation in $INSTALL_PREFIX"
        CONTINUE_INSTALL=1

    else

        echo "---------- Aborting..."
        exit;

    fi

fi

if [[ $CONTINUE_INSTALL == 0 ]]; then

    #################################################################################################
    # Install HomeBrew

    NONINTERACTIVE=1 /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    (echo; echo 'eval "$(/opt/homebrew/bin/brew shellenv)"') >> /Users/gilles/.zprofile
    eval "$(/opt/homebrew/bin/brew shellenv)"

fi

#################################################################################################
# Macports update

echo -e "\n"
echo "---------- Updating HomeBrew"
#brew -v selfupdate

if [[ $CONTINUE_INSTALL == 0 ]]; then

#    brew -v upgrade outdated
    echo -e "\n"

fi

#################################################################################################
# Dependencies build and installation

echo -e "\n"
echo "---------- Building digiKam dependencies with Macports"

echo -e "\n"

brew install \
             cmake \
             ccache \
             libpng \
             jpeg \
             libtiff \
             boost \
             eigen \
             mesa \
             gettext \
             gperf \
             libusb \
             libgphoto2 \
             jasper \
             little-cms2 \
             expat \
             libxml2 \
             libxslt \
             libical \
             bison \
             python-lxml \
             x265 \
             libde265 \
             libheif \
             aom \
             ffmpeg \
             wget \
             qt \
             qt-mariadb \
             opencv \
             imagemagick \
             jpeg-xl \
             libavif \
             fftw \
             exiv2 \
             lensfun

echo -e "\n"

#################################################################################################

# Create the build dir for the 3rdparty deps

if [ ! -d $BUILDING_DIR ] ; then

    mkdir $BUILDING_DIR

fi

if [ ! -d $DOWNLOAD_DIR ] ; then

    mkdir $DOWNLOAD_DIR

fi

cd $BUILDING_DIR

rm -rf $BUILDING_DIR/* || true

#cmake $ORIG_WD/../3rdparty \
#       -DCMAKE_INSTALL_PREFIX:PATH=$INSTALL_PREFIX \
#       -DINSTALL_ROOT=$INSTALL_PREFIX \
#       -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
#       -DKA_VERSION=$DK_KA_VERSION \
#       -DKP_VERSION=$DK_KP_VERSION \
#       -DKDE_VERSION=$DK_KDE_VERSION \
#       -DENABLE_QTVERSION=$DK_QTVERSION \
#       -Wno-dev
#
#cmake --build . --config RelWithDebInfo --target ext_libjxl      -- -j$CPU_CORES

#################################################################################################

export PATH=$ORIG_PATH

TerminateScript
