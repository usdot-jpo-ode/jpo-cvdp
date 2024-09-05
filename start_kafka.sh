#!/bin/bash

./stop_kafka.sh

# start kafka
docker compose -f docker-compose-kafka.yml up -d