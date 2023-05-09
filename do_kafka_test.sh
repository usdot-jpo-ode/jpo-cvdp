#!/bin/bash

KAFKA_CONTAINER_NAME=jpo-cvdp-kafka-1
MAP_FILE=data/I_80.edges
BSM_DATA_FILE=data/I_80_test.json
TIM_DATA_FILE=data/I_80_test_TIMS.json
PPM_CONTAINER_NAME=ppm_kafka
PPM_IMAGE_TAG=do-kafka-test-ppm-image
PPM_IMAGE_NAME=jpo-cvdp_ppm

setup() {
    # exit if DOCKER_HOST_IP is not set
    if [ -z $DOCKER_HOST_IP ]; then
        echo "[log] DOCKER_HOST_IP is not set. Exiting."
        exit 1
    fi

    if [ $(docker ps | grep $PPM_CONTAINER_NAME | wc -l) != "0" ]; then
        echo "[log] stopping existing PPM container"
        docker stop $PPM_CONTAINER_NAME > /dev/null
    fi
    docker rm -f $PPM_CONTAINER_NAME > /dev/null

    ./start_kafka.sh
}

waitForKafkaToCreateTopics() {
    # Wait until Kafka creates our topics.
    while true; do
        # if kafka container is not running, exit
        if [ $(docker ps | grep $KAFKA_CONTAINER_NAME | wc -l) == "0" ]; then
            echo "Kafka container '$KAFKA_CONTAINER_NAME' is not running. Exiting."
            ./stop_kafka.sh
            exit 1
        fi

        ltopics=$(docker exec -it $KAFKA_CONTAINER_NAME /opt/kafka/bin/kafka-topics.sh --list --zookeeper 172.17.0.1)

        # required topics:
        # - topic.FilteredOdeBsmJson
        # - topic.FilteredOdeTimJson
        # - topic.OdeBsmJson
        # - topic.OdeTimJson

        # use greps to check ltopics for required topics
        if [ $(echo $ltopics | grep "topic.FilteredOdeBsmJson" | wc -l) == "0" ]; then
            echo "[log] Kafka has not created topic 'topic.FilteredOdeBsmJson'"
        elif [ $(echo $ltopics | grep "topic.FilteredOdeTimJson" | wc -l) == "0" ]; then
            echo "[log] Kafka has not created topic 'topic.FilteredOdeTimJson'"
        elif [ $(echo $ltopics | grep "topic.OdeBsmJson" | wc -l) == "0" ]; then
            echo "[log] Kafka has not created topic 'topic.OdeBsmJson'"
        elif [ $(echo $ltopics | grep "topic.OdeTimJson" | wc -l) == "0" ]; then
            echo "[log] Kafka has not created topic 'topic.OdeTimJson'"
        else
            echo "[log] Kafka has created all required topics"
            break
        fi

        echo "[log] waiting for Kafka to create topics..."
        sleep 1
    done
}

buildPPMImage() {
    # build the PPM image
    echo "[log] building PPM image: "$PPM_IMAGE_NAME:$PPM_IMAGE_TAG
    docker build . -t $PPM_IMAGE_NAME:$PPM_IMAGE_TAG
}

run_tests() {
    echo "--- File Being Used ---"
    echo $MAP_FILE
    echo $BSM_DATA_FILE
    echo $TIM_DATA_FILE
    echo "-----------------"

    numberOfTests=10
    echo "Test 1/$numberOfTests"
    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c1.properties $BSM_DATA_FILE BSM 0
    echo ""
    echo ""

    echo "Test 2/$numberOfTests"
    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c2.properties $BSM_DATA_FILE BSM 10
    echo ""
    echo ""

    echo "Test 3/$numberOfTests"
    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c3.properties $BSM_DATA_FILE BSM 18
    echo ""
    echo ""

    echo "Test 4/$numberOfTests"
    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c4.properties $BSM_DATA_FILE BSM 23
    echo ""
    echo ""

    echo "Test 5/$numberOfTests"
    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c5.properties $BSM_DATA_FILE BSM 33
    echo ""
    echo ""

    echo "Test 6/$numberOfTests"
    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c6.properties $BSM_DATA_FILE BSM 43
    echo ""
    echo ""

    echo "Test 7/$numberOfTests"
    ./test-scripts/standalone.sh $MAP_FILE config/tim-test/c1.properties $TIM_DATA_FILE TIM 0
    echo ""
    echo ""

    echo "Test 8/$numberOfTests"
    ./test-scripts/standalone.sh $MAP_FILE config/tim-test/c2.properties $TIM_DATA_FILE TIM 10
    echo ""
    echo ""

    echo "Test 9/$numberOfTests"
    ./test-scripts/standalone.sh $MAP_FILE config/tim-test/c3.properties $TIM_DATA_FILE TIM 18
    echo ""
    echo ""

    echo "Test 10/$numberOfTests"
    ./test-scripts/standalone_multi.sh $MAP_FILE config/bsm-test/c6.properties config/tim-test/c3.properties $BSM_DATA_FILE $TIM_DATA_FILE 48 23
}

cleanup() {
    echo "[log] stopping Kafka"
    ./stop_kafka.sh
}

run() {
    numberOfSteps=5
    echo ""
    echo "Step 1/$numberOfSteps: Set up test environment"
    setup

    echo ""
    echo "Step 2/$numberOfSteps: Wait for Kafka to create topics"
    waitForKafkaToCreateTopics

    echo ""
    echo "Step 3/$numberOfSteps: Build PPM image"
    buildPPMImage

    echo ""
    echo "Step 4/$numberOfSteps: Run tests"
    run_tests

    echo ""
    echo "Step 5/$numberOfSteps: Cleanup"
    cleanup
}

run
