#!/bin/bash
if [ "$1" == "" ] || [ $# -gt 1 ]; then
        echo "Please specify package name"
        exit 0
fi

host=192.168.0.2

make package/$1/compile V=99
retVal=$?
if [ $retVal -eq 0 ]; then
    scp -P22 bin/packages/mips_24kc/base/$1_0.1-1_mips_24kc.ipk root@$host:/tmp
else
    echo "Build error"
fi
