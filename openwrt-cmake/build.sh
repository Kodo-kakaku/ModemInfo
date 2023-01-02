#!/bin/bash
if [ "$1" == "" ] || [ $# -gt 1 ]; then
        echo "Please specify package name"
        exit 0
fi
build=Debug
rm -rf build-x64-$build
cmake -DCMAKE_BUILD_TYPE=$build -H. -Bbuild-x64-$build package/$1/src/
cmake --build ./build-x64-$build/ --config $build
ls -la build-x64-$build/$1
