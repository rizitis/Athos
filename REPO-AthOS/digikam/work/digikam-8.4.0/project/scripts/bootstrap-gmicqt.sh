#!/bin/bash

# SPDX-FileCopyrightText: 2008-2024 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copy this script on root folder where are source code

# We will work on command line using MinGW compiler
export MAKEFILES_TYPE='Unix Makefiles'

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

cmake -G "$MAKEFILES_TYPE" . \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -DENABLE_ASAN=OFF \
      -DENABLE_SYSTEM_GMIC=OFF \
      -DGMIC_QT_HOST=digikam \
      -Wno-dev \
      ..

