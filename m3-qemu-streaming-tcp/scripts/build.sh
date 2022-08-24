#!/usr/bin/env bash

rm -rf build
mkdir -p build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../gcc-toolchain.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make

exit 0
