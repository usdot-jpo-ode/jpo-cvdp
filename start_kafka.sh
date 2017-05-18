#!/bin/bash

# Clone the kafka-docker repo.
git clone https://github.com/wurstmeister/kafka-docker.git

# Start the docker services.
docker-compose stop
docker-compose rm -f -v
docker-compose up --build -d
docker ps
