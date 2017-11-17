docker run -it -v %DOCKER_SHARED_VOLUME%:/ppm_data -e DOCKER_HOST_IP=%DOCKER_HOST_IP% -e PPM_CONFIG_FILE=%PPM_CONFIG_FILE% jpoode_ppm:latest /cvdi-stream/docker-test/ppm.sh
