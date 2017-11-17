#!/bin/sh
echo ${DOCKER_SHARED_VOLUME}
echo ${DOCKER_HOST_IP}

USAGE="start_ppm_ode_container.sh [MAP_FILE] [CONFIG]"

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

mkdir -p ${DOCKER_SHARED_VOLUME}

# Copy the road file to the docker test data.
cp $1 ${DOCKER_SHARED_VOLUME}/road_file.csv

# Copy the config to the test data.
cp $2 ${DOCKER_SHARED_VOLUME}/config.properties

docker run -it -v ${DOCKER_SHARED_VOLUME}:/ppm_data -e DOCKER_HOST_IP=${DOCKER_HOST_IP} -e PPM_CONFIG_FILE=${PPM_CONFIG_FILE} jpoode_ppm:latest /cvdi-stream/docker-test/ppm.sh
