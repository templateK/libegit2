#!/bin/bash

rm -rf ./build
mkdir -p build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/Volumes/Ramdisk/git2 \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH='/usr/local/opt/libiconv;/usr/local/opt/openssl;/usr/local/opt/libssh' ..
cmake --build .
