#!/bin/bash

# NOTE that this script will hang if the offsets are wrong. In other words,
# only use this for testing/demo; NOT production.

# There are three input files: ROAD_FILE, CONFIG, TEST_DATA.
# Offset is the offset in the topic that will be consumed and displayed in the
# output

PPM_CONTAINER_NAME=ppm_kafka
PPM_IMAGE_TAG=do-kafka-test-ppm-image
PPM_IMAGE_NAME=jpo-cvdp_ppm

startPPMContainer() {
    # Start the PPM in a new container.
    dockerHostIp=$DOCKER_HOST_IP

    # make sure ip can be pinged
    while true; do
        if ping -c 1 $dockerHostIp &> /dev/null; then
            break
        else
            echo "Docker host ip $dockerHostIp is not pingable. Exiting."
            exit 1
        fi
    done
    echo "Starting PPM in new container"
    docker run --name $PPM_CONTAINER_NAME --env DOCKER_HOST_IP=$dockerHostIp --env PPM_LOG_TO_CONSOLE=true --env PPM_LOG_TO_FILE=true -v /tmp/docker-test/data:/ppm_data -d -p '8080:8080' $PPM_IMAGE_NAME:$PPM_IMAGE_TAG /cvdi-stream/docker-test/ppm_standalone.sh

    # wait until container spins up
    while true; do
        if [ $(docker ps | grep $PPM_CONTAINER_NAME | wc -l) == "0" ]; then
            echo "PPM container '$PPM_CONTAINER_NAME' is not running. Exiting."
            exit 1
        fi

        container_logs=$(docker logs $PPM_CONTAINER_NAME 2>&1)
        indicator="BSMHandler::BSMHandler(): Constructor called"
        if [ $(echo $container_logs | grep "$indicator" | wc -l) != "0" ]; then
            echo "PPM container is ready"
            break
        fi

        sleep 1
    done

    container_logs=$(docker logs $PPM_CONTAINER_NAME 2>&1)
    if [ $(echo $container_logs | grep "Failed to make shape" | wc -l) != "0" ]; then
        echo "Warning: PPM failed to make shape."
    fi
}

stopPPMContainer() {
    if [ $(docker ps | grep $PPM_CONTAINER_NAME | wc -l) != "0" ]; then
        echo "Stopping existing PPM container"
        docker stop $PPM_CONTAINER_NAME > /dev/null
    fi
    docker rm -f $PPM_CONTAINER_NAME > /dev/null
}

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
echo "Running standalone test in $PPM_CONTAINER_NAME container with "$1 $2 $3 $4
echo "**************************"

startPPMContainer

if [ $4 = "BSM" ]; then
    docker exec $PPM_CONTAINER_NAME /cvdi-stream/docker-test/do_bsm_test.sh $OFFSET
elif [ $4 = "TIM" ]; then
    docker exec $PPM_CONTAINER_NAME /cvdi-stream/docker-test/do_tim_test.sh $OFFSET
fi

stopPPMContainer