#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib

broker=172.22.27.167:9092 #TODO: this should be the host machine

# Start the DI tool.
echo "[log] starting DI tool..."
/cvdi-stream-build/ppm -c /ppm_data/config.properties -o end
