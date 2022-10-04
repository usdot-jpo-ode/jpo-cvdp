#!/bin/sh
if [ -d build ] ; then 
    rm -r build
fi
mkdir build 
cd build
cmake ..
make
./ppm_tests