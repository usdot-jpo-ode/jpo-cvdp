#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib

broker=172.17.0.1:9092

# Start the DI tool.
/cvdi-stream-build/ppm -c /ppm_data/config.properties -o end
