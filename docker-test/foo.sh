#!/bin/bash
export LD_LIBRARY_PATH=/usr/local/lib

broker=172.17.0.1:9092

/cvdi-stream-build/kafka-test/kafka_tool -C -b 172.17.0.1:9092 -p 0 -t topic.OdeTimJson -e -o 0
