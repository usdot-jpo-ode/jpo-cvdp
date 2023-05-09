#!/bin/bash

# This script starts two PPM containers, one for BSMs and one for TIMs, in separate Docker containers.
# It first checks if all necessary input files exist, creates necessary directories, and copies the input
# files to those directories. It then starts the PPM containers, waits for them to spin up, produces test
# data, and finally stops the containers.

# This script should only be used for testing or demo purposes, as it may hang if the offsets are wrong. It also
# checks if the required configuration and test data files exist before proceeding with the test.

PPM_BSM_CONTAINER_NAME=ppm_bsm_kafka
PPM_TIM_CONTAINER_NAME=ppm_tim_kafka
PPM_IMAGE_TAG=do-kafka-test-ppm-image
PPM_IMAGE_NAME=jpo-cvdp_ppm
SECONDS_TO_WAIT_FOR_PPM_READINESS=20

USAGE="standalone_multi.sh [MAP_FILE] [BSM_CONFIG] [TIM_CONFIG] [BSM_TEST_FILE] [TIM_TEST_FILE] [BSM_OFFSET] [TIM_OFSET]"

startPPMContainer() {
    # Start the PPM in a new container.
    dockerHostIp=$DOCKER_HOST_IP
    PPM_CONTAINER_NAME=$1
    data_source=$2
    ppm_container_port=$3

    stopPPMContainer $PPM_CONTAINER_NAME

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
    docker run --name $PPM_CONTAINER_NAME --env DOCKER_HOST_IP=$dockerHostIp --env PPM_LOG_TO_CONSOLE=true --env PPM_LOG_TO_FILE=true -v $data_source:/ppm_data -d -p $ppm_container_port':8080' $PPM_IMAGE_NAME:$PPM_IMAGE_TAG /cvdi-stream/docker-test/ppm_standalone.sh

    echo "Giving $PPM_CONTAINER_NAME $SECONDS_TO_WAIT_FOR_PPM_READINESS seconds to spin up"
    sleep $SECONDS_TO_WAIT_FOR_PPM_READINESS

    if [ $(docker ps | grep $PPM_CONTAINER_NAME | wc -l) == "0" ]; then
        echo "PPM container '$PPM_CONTAINER_NAME' is not running. Exiting."
        exit 1
    fi

    container_logs=$(docker logs $PPM_CONTAINER_NAME 2>&1)
    if [ $(echo $container_logs | grep "Failed to make shape" | wc -l) != "0" ]; then
        echo "Warning: PPM failed to make shape."
    fi
}

stopPPMContainer() {
    PPM_CONTAINER_NAME=$1
    if [ $(docker ps | grep $PPM_CONTAINER_NAME | wc -l) != "0" ]; then
        echo "Stopping existing PPM container"
        docker stop $PPM_CONTAINER_NAME > /dev/null
    fi
    docker rm -f $PPM_CONTAINER_NAME > /dev/null
}

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

dockerHostIp=$DOCKER_HOST_IP
# Start the BSM PPM in a new container.
startPPMContainer $PPM_BSM_CONTAINER_NAME /tmp/docker-test/bsm-data 8080

# Start the TIM PPM in a new container.
startPPMContainer $PPM_TIM_CONTAINER_NAME /tmp/docker-test/tim-data 8081

# Produce the test data.
docker exec $PPM_BSM_CONTAINER_NAME /cvdi-stream/docker-test/do_bsm_test.sh $BSM_OFFSET
docker exec $PPM_TIM_CONTAINER_NAME /cvdi-stream/docker-test/do_tim_test.sh $TIM_OFFSET

stopPPMContainer $PPM_BSM_CONTAINER_NAME
stopPPMContainer $PPM_TIM_CONTAINER_NAME
