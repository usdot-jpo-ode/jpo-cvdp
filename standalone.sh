#!/bin/bash

# There are three input files: ROAD_FILE, CONFIG, TEST_DATA.

USAGE="standalone.sh [MAP_FILE] [CONFIG] [TEST_FILE]"

if [ -z $1 ] || [ ! -f $1 ]; then
    echo "Map file: "$1" not found!"
    echo $USAGE
    exit 1
fi

if [ -z $1 ] || [ ! -f $2 ]; then
    echo "Config file: "$2" not found!"
    echo $USAGE
    exit 1
fi

if [ -z $1 ] || [ ! -f $3 ]; then
    echo "Test file: "$3" not found!"
    echo $USAGE
    exit 1
fi

mkdir -p /tmp/docker-test/data

# Copy the road file to the docker test data.
cp $1 /tmp/docker-test/data/road_file.csv

# Copy the config to the test data.
# TODO replace map file line: sed -i '/TEXT_TO_BE_REPLACED/c\This line is removed by the admin.' /tmp/foo
cp $2 /tmp/docker-test/data/config.properties

# Copy the data.
cp $3 /tmp/docker-test/data/test.json

# Run the ppm module.
docker run --name ppm_kafka -v /tmp/docker-test/data:/ppm_data -it --rm -p '8080:8080' -d cvdigeofence_ppm:latest /cvdi-stream/docker-test/ppm.sh > /dev/null

# Give the server a chance to update.
sleep 2

# Produce the test data.
echo "**************************"
echo "Producing Raw BSMs..."
echo "**************************"
docker exec ppm_kafka /cvdi-stream/docker-test/producer.sh

# Give the server a chance to update.
sleep 2

# Consume the DI data.
echo "**************************"
echo "Consuming Filtered BSMs..."
echo "**************************"
docker exec ppm_kafka /cvdi-stream/docker-test/consumer.sh

# Stop the containter.
docker stop ppm_kafka > /dev/null
