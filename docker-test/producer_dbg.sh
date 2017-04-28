#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib

broker=172.17.0.1:9092

# Produce some BSMs.
cat /ppm_data/test.json | /cvdi-stream/docker-test/test_in.py | /cvdi-stream-build/kafka-test/kafka_tool -P -b $broker -p 0 -t j2735BsmRawJson
