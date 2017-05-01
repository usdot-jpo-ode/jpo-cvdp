#!/bin/bash

./start_kafka.sh

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
