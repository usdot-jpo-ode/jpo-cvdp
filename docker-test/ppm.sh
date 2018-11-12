#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib

# Start the DI tool.

/cvdi-stream-build/ppm -m /ppm_data/road_file.csv -c /ppm_data/${PPM_CONFIG_FILE} -b ${DOCKER_HOST_IP}:9092 

