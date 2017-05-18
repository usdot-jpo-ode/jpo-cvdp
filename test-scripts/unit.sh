#!/bin/bash
echo "**************************"
echo "Running unit tests."
echo "**************************"

# Start the PPM in a new container.
docker run --name ppm_unit -it --rm jpocvdp_ppm:latest /cvdi-stream/docker-test/ppm_unit.sh
