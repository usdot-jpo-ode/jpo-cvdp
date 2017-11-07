#!/bin/bash
export LD_LIBRARY_PATH=/usr/local/lib

broker=172.17.0.1:9092

echo "**************************"
echo "Producing Raw BSMs..."
echo "**************************"
cat /ppm_data/bsm_test.json | /cvdi-stream/docker-test/test_in.py | /cvdi-stream-build/kafka-test/kafka_tool -P -b $broker -p 0 -t topic.OdeBsmJson 2> priv.err

# Start the DI consumer.
offset=$1

echo "**************************"
echo "Consuming Filtered BSMs at offset "$offset "..." 
echo "**************************"

while true; do
    /cvdi-stream-build/kafka-test/kafka_tool -C -b $broker -p 0 -t topic.FilteredOdeBsmJson -e -o $offset 2> con.err | /cvdi-stream/docker-test/test_out.py > tmp.out

    lines=$(cat tmp.out | wc -l)

    if [[ $lines != "0" ]]; then 
        cat tmp.out

        break
    fi
done
