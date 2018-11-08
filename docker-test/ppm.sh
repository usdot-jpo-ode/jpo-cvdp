#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib

# Start the DI tool.

/cvdi-stream-build/ppm -c /cvdi-stream/config/${PPM_CONFIG_FILE} -m /cvdi-stream/config/${PPM_MAP_FILE} -b ${DOCKER_HOST_IP}:9092 

