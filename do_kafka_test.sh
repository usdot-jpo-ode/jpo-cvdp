#!/bin/bash

# This script tests the PPM against a kafka cluster. It sets up variables for container and input data
# file names. It starts a Kafka container using another script and checks that required topics are created.
# If the container or topics are missing, the script exits. It builds a Docker image using the current
# directory and specified name/tag. It runs a series of tests using a script with different properties
# and input data files, outputting results to the console. It stops the Kafka container after the tests
# are completed. The script performs five steps: set up the test environment, wait for Kafka to create
# topics, build the PPM image, run the tests, and clean up.

CYAN='\033[0;36m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

KAFKA_CONTAINER_NAME=jpo-cvdp-kafka-1
MAP_FILE=data/I_80.edges
BSM_DATA_FILE=data/I_80_test.json
TIM_DATA_FILE=data/I_80_test_TIMS.json
PPM_CONTAINER_NAME=ppm_kafka
PPM_IMAGE_TAG=do-kafka-test-ppm-image
PPM_IMAGE_NAME=jpo-cvdp_ppm

setup() {
    if [ -z $DOCKER_HOST_IP ]; then
        echo "DOCKER_HOST_IP is not set. Exiting."
        exit 1
    fi

    ./start_kafka.sh
}

waitForKafkaToCreateTopics() {
    while true; do
        if [ $(docker ps | grep $KAFKA_CONTAINER_NAME | wc -l) == "0" ]; then
            echo "Kafka container '$KAFKA_CONTAINER_NAME' is not running. Exiting."
            ./stop_kafka.sh
            exit 1
        fi

        ltopics=$(docker exec -it $KAFKA_CONTAINER_NAME /opt/kafka/bin/kafka-topics.sh --list --zookeeper 172.17.0.1)
        allTopicsCreated=true
        if [ $(echo $ltopics | grep "topic.FilteredOdeBsmJson" | wc -l) == "0" ]; then
            allTopicsCreated=false
        elif [ $(echo $ltopics | grep "topic.FilteredOdeTimJson" | wc -l) == "0" ]; then
            allTopicsCreated=false
        elif [ $(echo $ltopics | grep "topic.OdeBsmJson" | wc -l) == "0" ]; then
            allTopicsCreated=false
        elif [ $(echo $ltopics | grep "topic.OdeTimJson" | wc -l) == "0" ]; then
            allTopicsCreated=false
        fi
        
        if [ $allTopicsCreated == true ]; then
            echo "Kafka has created all required topics"
            break
        fi

        sleep 1
    done
}

buildPPMImage() {
    docker build . -t $PPM_IMAGE_NAME:$PPM_IMAGE_TAG
}

run_tests() {
    echo "--- File Being Used ---"
    echo $MAP_FILE
    echo $BSM_DATA_FILE
    echo $TIM_DATA_FILE
    echo "-----------------"

    numberOfTests=10
    echo -e $YELLOW"Test 1/$numberOfTests"$NC
    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c1.properties $BSM_DATA_FILE BSM 0
    echo ""
    echo ""

    echo -e $YELLOW"Test 2/$numberOfTests"$NC
    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c2.properties $BSM_DATA_FILE BSM 10
    echo ""
    echo ""

    echo -e $YELLOW"Test 3/$numberOfTests"$NC
    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c3.properties $BSM_DATA_FILE BSM 18
    echo ""
    echo ""

    echo -e $YELLOW"Test 4/$numberOfTests"$NC
    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c4.properties $BSM_DATA_FILE BSM 23
    echo ""
    echo ""

    echo -e $YELLOW"Test 5/$numberOfTests"$NC
    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c5.properties $BSM_DATA_FILE BSM 33
    echo ""
    echo ""

    echo -e $YELLOW"Test 6/$numberOfTests"$NC
    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c6.properties $BSM_DATA_FILE BSM 43
    echo ""
    echo ""

    echo -e $YELLOW"Test 7/$numberOfTests"$NC
    ./test-scripts/standalone.sh $MAP_FILE config/tim-test/c1.properties $TIM_DATA_FILE TIM 0
    echo ""
    echo ""

    echo -e $YELLOW"Test 8/$numberOfTests"$NC
    ./test-scripts/standalone.sh $MAP_FILE config/tim-test/c2.properties $TIM_DATA_FILE TIM 10
    echo ""
    echo ""

    echo -e $YELLOW"Test 9/$numberOfTests"$NC
    ./test-scripts/standalone.sh $MAP_FILE config/tim-test/c3.properties $TIM_DATA_FILE TIM 18
    echo ""
    echo ""

    echo -e $YELLOW"Test 10/$numberOfTests (2 tests in one)"$NC
    ./test-scripts/standalone_multi.sh $MAP_FILE config/bsm-test/c6.properties config/tim-test/c3.properties $BSM_DATA_FILE $TIM_DATA_FILE 48 23
}

cleanup() {
    echo "[log] stopping Kafka"
    ./stop_kafka.sh
}

run() {
    numberOfSteps=5
    echo ""
    echo -e $CYAN"Step 1/$numberOfSteps: Set up test environment"$NC
    setup

    echo ""
    echo -e $CYAN"Step 2/$numberOfSteps: Wait for Kafka to create topics"$NC
    waitForKafkaToCreateTopics

    echo ""
    echo -e $CYAN"Step 3/$numberOfSteps: Build PPM image"$NC
    buildPPMImage

    echo ""
    echo -e $CYAN"Step 4/$numberOfSteps: Run tests"$NC
    run_tests

    echo ""
    echo -e $CYAN"Step 5/$numberOfSteps: Cleanup"$NC
    cleanup
}

run
