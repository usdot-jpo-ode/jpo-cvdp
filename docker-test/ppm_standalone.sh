#!/bin/sh

# This script starts the PPM tool with the specified configuration file, using the DOCKER_HOST_IP
# environment variable to connect to a Kafka broker.

export LD_LIBRARY_PATH=/usr/local/lib

if [ -z "$DOCKER_HOST_IP" ]; then
    echo "DOCKER_HOST_IP is not set. Exiting."
    exit 1
fi

# Start the DI tool.
/cvdi-stream-build/ppm -c /ppm_data/config.properties -b ${DOCKER_HOST_IP}:9092 -o end 
