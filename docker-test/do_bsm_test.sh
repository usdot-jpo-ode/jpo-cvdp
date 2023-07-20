#!/bin/bash

# This script produces and consumes messages from Kafka topics. It reads a JSON file containing raw
# BSM data, processes it with a Python script, and then sends the output to a Kafka topic. Then it
# consumes the filtered messages from the Kafka topic, using a specified offset, and checks if
# any messages were received. If no messages were received after a certain number of attempts, the
# script exits with an error message. Otherwise, the script exits with a success message.

export LD_LIBRARY_PATH=/usr/local/lib

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

broker=$DOCKER_HOST_IP:9092

echo "**************************"
echo "Producing Raw BSMs..."
echo "**************************"
cat /ppm_data/bsm_test.json | /cvdi-stream/docker-test/test_in.py | /cvdi-stream-build/kafka-test/kafka_tool -P -b $broker -p 0 -t topic.OdeBsmJson 2> priv.err

# Start the DI consumer.
offset=$1

echo "**************************"
echo "Consuming Filtered BSMs at offset "$offset "..." 
echo "**************************"

attempts=0
max_attempts=5
while true; do
    attempts=$((attempts+1))

    timeout 5 /cvdi-stream-build/kafka-test/kafka_tool -C -b $broker -p 0 -t topic.FilteredOdeBsmJson -e -o $offset 2> con.err | /cvdi-stream/docker-test/test_out.py > tmp.out
    if [[ $? != 0 ]]; then
        echo "Error: Kafka consumer timed out."
        echo "~~~~~~~~~~~~~~~~~~~~~~~~~~"
        echo -e $RED"TEST FAILED!"$NC
        echo "~~~~~~~~~~~~~~~~~~~~~~~~~~"
        exit 1
    fi

    lines=$(cat tmp.out | wc -l)
    if [[ $lines != "0" ]]; then 
        cat tmp.out
        break
    else
        if [[ $attempts > $max_attempts ]]; then
            echo "No data received after $max_attempts attempts. Exiting..."
            echo "~~~~~~~~~~~~~~~~~~~~~~~~~~"
            echo -e $RED"TEST FAILED!"$NC
            echo "~~~~~~~~~~~~~~~~~~~~~~~~~~"
            exit 1
        fi
    fi
done
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~"
echo -e $GREEN"TEST PASSED!"$NC
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~"