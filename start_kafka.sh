#!/bin/bash

# Start the docker services.
docker-compose stop
docker-compose rm -f -v
docker-compose up --build -d
docker ps
