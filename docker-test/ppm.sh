#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib

# Start the DI tool.
/cvdi-stream-build/bsmjson_privacy -g 0 -b ${DOCKER_HOST_IP}:9092 -c /ppm_data/config/ppm.properties 2> priv.err
