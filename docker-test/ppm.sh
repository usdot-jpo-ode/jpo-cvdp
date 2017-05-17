#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib

broker=172.17.0.1:9092

# Start the DI tool.
# /cvdi-stream-build/bsmjson_privacy -g 0 -b $broker -c /ppm_data/config.properties 2> priv.err
/cvdi-stream-build/ppm -g 0 -b $broker -c /ppm_data/config.properties 2> priv.err
