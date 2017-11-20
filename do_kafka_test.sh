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
BSM_DATA_FILE=data/I_80_test.json
TIM_DATA_FILE=data/I_80_test_TIMS.json

echo $MAP_FILE
echo $BSM_DATA_FILE
echo $TIM_DATA_FILE

./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c1.properties $BSM_DATA_FILE BSM 0
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c2.properties $BSM_DATA_FILE BSM 10
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c3.properties $BSM_DATA_FILE BSM 18
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c4.properties $BSM_DATA_FILE BSM 23
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c5.properties $BSM_DATA_FILE BSM 33
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/bsm-test/c6.properties $BSM_DATA_FILE BSM 43
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/tim-test/c1.properties $TIM_DATA_FILE TIM 0
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/tim-test/c2.properties $TIM_DATA_FILE TIM 10
echo ""
echo ""

sleep 1
./test-scripts/standalone.sh $MAP_FILE config/tim-test/c3.properties $TIM_DATA_FILE TIM 18
echo ""
echo ""

sleep 1
./test-scripts/standalone_multi.sh $MAP_FILE config/bsm-test/c6.properties config/tim-test/c3.properties $BSM_DATA_FILE $TIM_DATA_FILE 48 23
