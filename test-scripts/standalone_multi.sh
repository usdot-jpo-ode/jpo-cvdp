#!/bin/bash

# NOTE that this script will hang if the offsets are wrong. In other words,
# only use this for testing/demo; NOT production.

# Test two ppm modules at once, as two docker containers.
USAGE="standalone_multi.sh [MAP_FILE] [BSM_CONFIG] [TIM_CONFIG] [BSM_TEST_FILE] [TIM_TEST_FILE] [BSM_OFFSET] [TIM_OFSET]"

if [ -z $1 ] || [ ! -f $1 ]; then
    echo "Map file: "$1" not found!"
    echo $USAGE
    exit 1
fi

if [ -z $2 ] || [ ! -f $2 ]; then
    echo "BSM config file: "$2" not found!"
    echo $USAGE
    exit 1
fi

if [ -z $3 ] || [ ! -f $3 ]; then
    echo "TIM config file: "$3" not found!"
    echo $USAGE
    exit 1
fi

if [ -z $4 ] || [ ! -f $4 ]; then
    echo "BSM test file: "$4" not found!"
    echo $USAGE
    exit 1
fi

if [ -z $5 ] || [ ! -f $5 ]; then
    echo "TIM test file: "$5" not found!"
    echo $USAGE
    exit 1
fi

if [ -z $6 ]; then
    BSM_OFFSET=0
else
    BSM_OFFSET=$6
fi

if [ -z $7 ]; then
    TIM_OFFSET=0
else
    TIM_OFFSET=$7
fi

mkdir -p /tmp/docker-test/bsm-data
mkdir -p /tmp/docker-test/tim-data

# Copy the road files to the docker test data.
cp $1 /tmp/docker-test/bsm-data/road_file.csv
cp $1 /tmp/docker-test/tim-data/road_file.csv

# Copy the configs to the test data.
# TODO replace map file line: sed -i '/TEXT_TO_BE_REPLACED/c\This line is removed by the admin.' /tmp/foo
cp $2 /tmp/docker-test/bsm-data/config.properties
cp $3 /tmp/docker-test/tim-data/config.properties

# Copy the data.
cp $4 /tmp/docker-test/bsm-data/bsm_test.json
cp $5 /tmp/docker-test/tim-data/tim_test.json

echo "**************************"
echo "Running standalone multi PPM test with "$1 $2 $3 $4 $5 $6 $7
echo "**************************"

# Start the BSM PPM in a new container.
docker run --name ppm_bsm_kafka -v /tmp/docker-test/bsm-data:/ppm_data -it --rm -p '8080:8080' -d jpocvdp_ppm:latest /cvdi-stream/docker-test/ppm_standalone.sh > /dev/null
# Start the TIM PPM in a new container.
docker run --name ppm_tim_kafka -v /tmp/docker-test/tim-data:/ppm_data -it --rm -p '8081:8080' -d jpocvdp_ppm:latest /cvdi-stream/docker-test/ppm_standalone.sh > /dev/null

sleep 10

# Produce the test data.
docker exec ppm_bsm_kafka /cvdi-stream/docker-test/do_bsm_test.sh $BSM_OFFSET
docker exec ppm_tim_kafka /cvdi-stream/docker-test/do_tim_test.sh $TIM_OFFSET

docker stop ppm_bsm_kafka > /dev/null
docker stop ppm_tim_kafka > /dev/null
