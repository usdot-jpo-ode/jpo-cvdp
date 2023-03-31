#!/bin/sh

# check if REDACTION_PROPERTIES_PATH is set
if [ -z "$REDACTION_PROPERTIES_PATH" ] ; then
    echo "REDACTION_PROPERTIES_PATH is not set"
    exit 1
fi

# remove build directory if it exists
if [ -d build ] ; then 
    rm -r build
fi

# build
mkdir build 
cd build
cmake ..
make

# run unit tests
./ppm_tests