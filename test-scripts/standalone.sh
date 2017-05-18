#!/bin/bash

# There are three input files: ROAD_FILE, CONFIG, TEST_DATA.
# Offset is the offset in the topic that will be consumed and displayed in the
# output
USAGE="standalone.sh [MAP_FILE] [CONFIG] [TEST_FILE] [OFFSET]"

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

if [ -z $4 ]; then
    OFFSET=0
else
    OFFSET=$4
fi

mkdir -p /tmp/docker-test/data

# Copy the road file to the docker test data.
cp $1 /tmp/docker-test/data/road_file.csv

# Copy the config to the test data.
# TODO replace map file line: sed -i '/TEXT_TO_BE_REPLACED/c\This line is removed by the admin.' /tmp/foo
cp $2 /tmp/docker-test/data/config.properties

# Copy the data.
cp $3 /tmp/docker-test/data/test.json

echo "**************************"
echo "Running standalone test with "$1 $2 $3
echo "**************************"

#docker stop ppm_kafka > /dev/null
#docker rm ppm_kafka > /dev/null

# Start the PPM in a new container.
docker run --name ppm_kafka -v /tmp/docker-test/data:/ppm_data -it --rm -p '8080:8080' -d jpocvdp_ppm:latest /cvdi-stream/docker-test/ppm_standalone.sh > /dev/null

sleep 10

# Produce the test data.
docker exec ppm_kafka /cvdi-stream/docker-test/do_test.sh $OFFSET
docker stop ppm_kafka > /dev/null
