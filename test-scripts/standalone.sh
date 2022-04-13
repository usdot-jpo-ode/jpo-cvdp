#!/bin/bash

# NOTE that this script will hang if the offsets are wrong. In other words,
# only use this for testing/demo; NOT production.

# There are three input files: ROAD_FILE, CONFIG, TEST_DATA.
# Offset is the offset in the topic that will be consumed and displayed in the
# output
USAGE="standalone.sh [MAP_FILE] [CONFIG] [TEST_FILE] [BSM | TIM] [OFFSET]"

if [ -z $1 ] || [ ! -f $1 ]; then
    echo "Map file: "$1" not found!"
    echo $USAGE
    exit 1
fi

if [ -z $2 ] || [ ! -f $2 ]; then
    echo "Config file: "$2" not found!"
    echo $USAGE
    exit 1
fi

if [ -z $3 ] || [ ! -f $3 ]; then
    echo "Test file: "$3" not found!"
    echo $USAGE
    exit 1
fi

if [ -z $4 ]; then
    echo "Must include type (BSM or TIM)!"
    echo $USAGE
    exit 1
fi

if [ -z $5 ]; then
    OFFSET=0
else
    OFFSET=$5
fi

mkdir -p /tmp/docker-test/data

# Copy the road file to the docker test data.
cp $1 /tmp/docker-test/data/road_file.csv

# Copy the config to the test data.
# TODO replace map file line: sed -i '/TEXT_TO_BE_REPLACED/c\This line is removed by the admin.' /tmp/foo
cp $2 /tmp/docker-test/data/config.properties

# Copy the data.
if [ $4 = "BSM" ]; then
    cp $3 /tmp/docker-test/data/bsm_test.json
elif [ $4 = "TIM" ]; then
    cp $3 /tmp/docker-test/data/tim_test.json
else
    echo "Type must be BSM or TIM!"
fi

echo "**************************"
echo "Running standalone test with "$1 $2 $3 $4
echo "**************************"

#docker stop ppm_kafka > /dev/null
#docker rm ppm_kafka > /dev/null

# Start the PPM in a new container.
docker run --name ppm_kafka -v /tmp/docker-test/data:/ppm_data -it --rm -p '8080:8080' -d jpocvdp_ppm:latest /cvdi-stream/docker-test/ppm_standalone.sh > /dev/null

sleep 10

if [ $4 = "BSM" ]; then
    docker exec ppm_kafka /cvdi-stream/docker-test/do_bsm_test.sh $OFFSET
    # Produce the test data.
elif [ $4 = "TIM" ]; then
    docker exec ppm_kafka /cvdi-stream/docker-test/do_tim_test.sh $OFFSET
    # Produce the test data.
fi

docker stop ppm_kafka > /dev/null
