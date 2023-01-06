#!/bin/bash
if [ "$1" == "" ] || [ $# -gt 1 ]; then
        echo "Please specify package name"
        exit 0
fi
cd ~/openwrt
host=192.168.0.2
export STAGING_DIR=~/openwrt/staging_dir/
build=Release
rm -rf build-mips-$build
cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/Toolchain-mips.cmake -DCMAKE_BUILD_TYPE=$build -H. -Bbuild-mips-$build package/$1/src/
cmake --build ./build-mips-$build/ --config $build
scp -P22 build-mips-$build/$1 root@$host:/tmp
