#!/bin/bash
./start_kafka.sh

# Wait until Kafka creates our topics.
while true; do
    ntopics=$(docker exec jpocvdp_kafka_1 /opt/kafka/bin/kafka-topics.sh --list --zookeeper 172.17.0.1 | wc -l)

    if [[ $ntopics == "4" ]]; then 
        echo 'Found 4 topics:'
        docker exec jpocvdp_kafka_1 /opt/kafka/bin/kafka-topics.sh --list --zookeeper 172.17.0.1 2> /dev/null
        
        break   
    fi

    sleep 1
done

MAP_FILE=data/I_80.edges
DATA_FILE=data/I_80_test.json

echo $MAP_FILE
echo $DATA_FILE

./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c1.properties $DATA_FILE BSM 0
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c2.properties $DATA_FILE BSM 10
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c3.properties $DATA_FILE BSM 18
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c4.properties $DATA_FILE BSM 23
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c5.properties $DATA_FILE BSM 33
echo ""
echo ""

DATA_FILE=data/I_80_test_TIMS.json

echo $MAP_FILE
echo $DATA_FILE

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/tim-test/c1.properties $DATA_FILE TIM 0
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/tim-test/c2.properties $DATA_FILE TIM 10
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/tim-test/c3.properties $DATA_FILE TIM 18
