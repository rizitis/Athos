#!/bin/bash

# SPDX-FileCopyrightText: 2008-2024 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copy this script on root folder where are source code

#export VERBOSE=1

# We will work on command line using MinGW compiler
export MAKEFILES_TYPE='Unix Makefiles'

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

export Options='-DCLAZY_AST_MATCHERS_CRASH_WORKAROUND=ON \
                -DLINK_CLAZY_TO_LLVM=ON \
                -DAPPIMAGE_HACK=OFF \
                -DCLAZY_MAN_PAGE=OFF'

cmake -G "$MAKEFILES_TYPE" . \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -Wno-dev \
      $Options \
      ..

