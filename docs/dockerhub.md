# jpo-cvdp

## GitHub Repository Link
https://github.com/usdot-jpo-ode/jpo-cvdp

## Purpose
The purpose of the jpo-cvdp program is to filter messages based on location/speed and redact personal identifiable information (PII) from processed messages.

## How to pull the latest image
The latest image can be pulled using the following command:
> docker pull usdotjpoode/jpo-cvdp:develop

## Required environment variables
The image expects the following environment variables to be set:
- DOCKER_HOST_IP
- DOCKER_SHARED_VOLUME
- PPM_CONFIG_FILE
- REDACTION_PROPERTIES_PATH

## Required files in `ppm_data` mounted directory
- fieldsToRedact.txt
- I_80.edges
- ppmBsm.properties

## Direct Dependencies
The image will fail to start up if the following containers are not already present:
- Kafka
- Zookeeper (relied on by Kafka)

## Indirect Dependencies
The image will not receive messages to process if the ODE is not running.

## Example docker-compose.yml with direct dependencies:
```
version: '2'
services:
  zookeeper:
    image: wurstmeister/zookeeper
    ports:
      - "2181:2181"

  kafka:
    image: wurstmeister/kafka
    ports:
      - "9092:9092"
    environment:
      KAFKA_ADVERTISED_HOST_NAME: ${DOCKER_HOST_IP}
      KAFKA_ZOOKEEPER_CONNECT: zookeeper:2181
      KAFKA_CREATE_TOPICS: "topic.OdeBsmJson:1:1,topic.FilteredOdeBsmJson:1:1,topic.OdeTimJson:1:1,topic.FilteredOdeTimJson:1:1"
    volumes:
      - /var/run/docker.sock:/var/run/docker.sock

  ppm:
    image: usdotjpoode/jpo-cvdp:release_q3
    environment:
      # required
      DOCKER_HOST_IP: ${DOCKER_HOST_IP}
      PPM_CONFIG_FILE: ${PPM_CONFIG_FILE}
      REDACTION_PROPERTIES_PATH: ${REDACTION_PROPERTIES_PATH}
      # optional
      PPM_LOG_TO_FILE: "false"
      PPM_LOG_TO_CONSOLE: ${PPM_LOG_TO_CONSOLE}
      RPM_DEBUG: "false"
      PPM_LOG_LEVEL: ${PPM_LOG_LEVEL}
    depends_on:
      - kafka
    volumes:
      - ${DOCKER_SHARED_VOLUME_WINDOWS}:/ppm_data
```

## Expected startup output
The latest logs should look like this:
```
jpo-cvdp-ppm-1        | [231109 22:20:14.700649] [info] Waiting for more BSMs from the ODE producer.
jpo-cvdp-ppm-1        | [231109 22:20:15.201389] [info] Waiting for more BSMs from the ODE producer.
jpo-cvdp-ppm-1        | [231109 22:20:15.701666] [info] Waiting for more BSMs from the ODE producer.
jpo-cvdp-ppm-1        | [231109 22:20:16.202104] [info] Waiting for more BSMs from the ODE producer.
jpo-cvdp-ppm-1        | [231109 22:20:16.702451] [info] Waiting for more BSMs from the ODE producer.
jpo-cvdp-ppm-1        | [231109 22:20:17.202653] [info] Waiting for more BSMs from the ODE producer.
```
