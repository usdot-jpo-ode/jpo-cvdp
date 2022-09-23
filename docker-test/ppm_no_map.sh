#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib

# Start the DI tool.

/cvdi-stream-build/ppm -c /ppm_data/${PPM_CONFIG_FILE} -b ${DOCKER_HOST_IP}:9092 -D ~/logs/

