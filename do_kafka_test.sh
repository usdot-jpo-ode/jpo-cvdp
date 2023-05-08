#!/bin/bash

KAFKA_CONTAINER_NAME=jpo-cvdp-kafka-1
MAP_FILE=data/I_80.edges
BSM_DATA_FILE=data/I_80_test.json
TIM_DATA_FILE=data/I_80_test_TIMS.json

setup() {
    echo "[log] setting up test environment"
    ./start_kafka.sh
}

waitForKafkaToCreateTopics() {
    # Wait until Kafka creates our topics.
    while true; do
        # if kafka container is not running, exit
        if [[ $(docker ps | grep $KAFKA_CONTAINER_NAME | wc -l) == "0" ]]; then
            echo "Kafka container '$KAFKA_CONTAINER_NAME' is not running. Exiting."
            ./stop_kafka.sh
            exit 1
        fi

        ntopics=$(docker exec -it $KAFKA_CONTAINER_NAME /opt/kafka/bin/kafka-topics.sh --list --zookeeper 172.17.0.1 | wc -l)

        expected_topics=4
        if [[ $ntopics == $expected_topics ]]; then 
            echo '[log] found '$expected_topics' topics as expected:'
            docker exec -it $KAFKA_CONTAINER_NAME /opt/kafka/bin/kafka-topics.sh --list --zookeeper 172.17.0.1 2> /dev/null
            
            break
        elif [[ $ntopics == "0" ]]; then
            echo '[log] no topics found'
        else
            echo '[log] found '$ntopics'/'$expected_topics' topics'
        fi

        echo "[log] waiting for Kafka to create topics..."
        sleep 1
    done
}

buildPPMImage() {
    # build the PPM image
    tag=do-kafka-test-ppm-image
    PPM_IMAGE_NAME=jpo-cvdp_ppm
    echo "[log] building PPM image: "$PPM_IMAGE_NAME:$tag
    docker build . -t $PPM_IMAGE_NAME:$tag
}


startPPMContainer() {
    # Start the PPM in a new container.
    PPM_CONTAINER_NAME=ppm_kafka
    if [[ $(docker ps | grep $PPM_CONTAINER_NAME | wc -l) != "0" ]]; then
        echo "[log] stopping existing PPM container"
        docker stop $PPM_CONTAINER_NAME > /dev/null
    fi
    echo "[log] starting PPM in new container"
    docker rm -f $PPM_CONTAINER_NAME > /dev/null
    dockerHostIp=$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $KAFKA_CONTAINER_NAME)
    echo "[log] docker host ip: '$dockerHostIp'"
    docker run --name $PPM_CONTAINER_NAME -v /tmp/docker-test/data:/ppm_data -d -p '8080:8080' $PPM_IMAGE_NAME:$tag /cvdi-stream/docker-test/ppm_standalone.sh --env DOCKER_HOST_IP=$dockerHostIp --env PPM_LOG_TO_CONSOLE=true --env PPM_LOG_TO_FILE=true

    # wait until container spins up
    while true; do
        if [[ $(docker ps | grep $PPM_CONTAINER_NAME | wc -l) == "0" ]]; then
            echo "PPM container '$PPM_CONTAINER_NAME' is not running. Exiting."
            exit 1
        fi

        nlines=$(docker logs $PPM_CONTAINER_NAME 2>&1 | wc -l)
        if [[ $nlines == "0" ]]; then
            echo "[log] waiting for PPM to start..."
        else
            echo "--- PPM LOGS ---"
            docker logs $PPM_CONTAINER_NAME
            echo "----------------"
            break
        fi

        sleep 1
    done
}

run_tests() {
    echo $MAP_FILE
    echo $BSM_DATA_FILE
    echo $TIM_DATA_FILE

    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c1.properties $BSM_DATA_FILE BSM 0
    echo ""
    echo ""

    # sleep 1
    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c2.properties $BSM_DATA_FILE BSM 10
    echo ""
    echo ""

    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c3.properties $BSM_DATA_FILE BSM 18
    echo ""
    echo ""

    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c4.properties $BSM_DATA_FILE BSM 23
    echo ""
    echo ""

    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c5.properties $BSM_DATA_FILE BSM 33
    echo ""
    echo ""

    ./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c6.properties $BSM_DATA_FILE BSM 43
    echo ""
    echo ""

    ./test-scripts/standalone.sh $MAP_FILE config/tim-test/c1.properties $TIM_DATA_FILE TIM 0
    echo ""
    echo ""

    ./test-scripts/standalone.sh $MAP_FILE config/tim-test/c2.properties $TIM_DATA_FILE TIM 10
    echo ""
    echo ""

    ./test-scripts/standalone.sh $MAP_FILE config/tim-test/c3.properties $TIM_DATA_FILE TIM 18
    echo ""
    echo ""

    ./test-scripts/standalone_multi.sh $MAP_FILE config/bsm-test/c6.properties config/tim-test/c3.properties $BSM_DATA_FILE $TIM_DATA_FILE 48 23
}

cleanup() {
    echo "[log] stopping PPM"
    docker stop $PPM_CONTAINER_NAME > /dev/null
    echo "[log] stopping Kafka"
    ./stop_kafka.sh
}

run() {
    echo ""
    echo "Step 1/6: Set up test environment"
    setup

    echo ""
    echo "Step 2/6: Wait for Kafka to create topics"
    waitForKafkaToCreateTopics

    echo ""
    echo "Step 3/6: Build PPM image"
    buildPPMImage

    echo ""
    echo "Step 4/6: Start PPM container"
    startPPMContainer

    echo ""
    echo "Step 5/6: Run tests"
    run_tests

    echo ""
    echo "Step 6/6: Cleanup"
    cleanup
}

run
