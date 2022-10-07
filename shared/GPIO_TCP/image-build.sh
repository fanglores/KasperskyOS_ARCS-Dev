#!/bin/bash

BUILD=$PWD/build
mkdir -p $BUILD && cd $BUILD

export LANG=C
export TARGET="aarch64-kos"
export PKG_CONFIG=""
export SDK_PREFIX="/opt/KasperskyOS-Community-Edition-1.1.0.23"
BUILD_SIM_TARGET="y"
export PATH="$SDK_PREFIX/toolchain/bin:$PATH"

BUILD_HOST=$BUILD 

cd $BUILD && cmake -G "Unix Makefiles" \
      -D CMAKE_BUILD_TYPE:STRING=Debug \
      -D CMAKE_INSTALL_PREFIX:STRING=$BUILD/../install \
      -D CMAKE_FIND_ROOT_PATH=$([[ -f ../additional_cmake_find_root.txt ]] && cat ../additional\
_cmake_find_root.txt)$SDK_PREFIX/sysroot-aarch6-kos \
      -D CMAKE_TOOLCHAIN_FILE=$SDK_PREFIX/toolchain/share/toolchain-$TARGET.cmake \
      ../ && make kos-image
