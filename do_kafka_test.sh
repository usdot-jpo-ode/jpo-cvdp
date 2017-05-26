#!/bin/bash
./start_kafka.sh

# Wait until Kafka creates our topics.
while true; do
    ntopics=$(docker exec jpocvdp_kafka_1 /opt/kafka/bin/kafka-topics.sh --list --zookeeper 172.17.0.1 | wc -l)

    if [[ $ntopics == "2" ]]; then 
        echo 'Found 2 topics:'
        docker exec jpocvdp_kafka_1 /opt/kafka/bin/kafka-topics.sh --list --zookeeper 172.17.0.1 2> /dev/null
        
        break   
    fi

    sleep 1
done

MAP_FILE=data/I_80.edges
DATA_FILE=data/I_80_test.json

./test-scripts/standalone.sh $MAP_FILE config/test/c1.properties $DATA_FILE 0
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/test/c2.properties $DATA_FILE 10
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/test/c3.properties $DATA_FILE 18
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/test/c4.properties $DATA_FILE 23
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/test/c5.properties $DATA_FILE 33
