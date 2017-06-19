# Installation and Setup

The following instructions represent the "hard" way to install and test the PPM. A docker image can be built to make
this easier: [Using the Docker Container](#using-the-docker-container). *The directions that follow were developed for a clean installation of Ubuntu.*

## 1. Install [Git](https://git-scm.com/)

```bash
$ sudo apt install git
```

## 2. Install Oracle’s Java

```bash
$ sudo add-apt-repository -y ppa:webupd8team/java
$ sudo apt update
$ sudo apt install oracle-java8-installer -y
$ sudo java -version
```

## 3. Install [CMake](https://cmake.org) to build the PPM

```bash
$ sudo apt install cmake
```

## 4. Install [Docker](https://www.docker.com)

- When following the website instructions, setup the Docker repos and follow the Linux post-install instructions.
- The CE version seems to work fine.
- [Docker installation instructions](https://docs.docker.com/engine/installation/linux/ubuntu/#install-using-the-repository)
- *ORNL specific, but may apply to others with organizational security*
    - Correct for internal Google DNS blocking
    - As root (`$ sudo su`), create a `daemon.json` file in the `/etc/docker` directory that contains the following information:
```bash
          {
              "debug": true,
              "default-runtime": "runc",
              "dns": ["160.91.126.23","160.91.126.28”],
              "icc": true,
              "insecure-registries": [],
              "ip": "0.0.0.0",
              "log-driver": "json-file",
              "log-level": "info",
              "max-concurrent-downloads": 3,
              "max-concurrent-uploads": 5,
              "oom-score-adjust": -500
          }
```
- NOTE: The DNS IP addresses are ORNL specific.

## 5. Restart the docker daemon to consume the new configuration file.

```bash
$ service docker stop
$ service docker start
```

## 6. Check the configuration using the command below to confirm the updates above are taken if needed:

```bash
$ docker info
```

## 7. Install Docker Compose
- Comprehensive instructions can be found on this [website](https://www.digitalocean.com/community/tutorials/how-to-install-docker-compose-on-ubuntu-16-04)
- Follow steps 1 and 2.

## 8. Create a base directory from which to install all the necessary components to test the PPM.

```bash
$ export BASE_PPM_DIR=~/some/dir/you/want/to/put/this/stuff
```

## 9. Install [`kafka-docker`](https://github.com/wurstmeister/kafka-docker) so kafka and zookeeper can run in a separate container.

- Get your host IP address. The address is usually listed under an ethernet adapter, e.g., `en<number>`.

```bash
$ ifconfig
$ export DOCKER_HOST_IP=<HOST IP>
```
- Get the kafka and zookeeper images.

```bash
$ cd $BASE_PPM_DIR
$ git clone https://github.com/wurstmeister/kafka-docker.git
$ cd kafka-docker
$ vim docker-compose.yml	                        // Set karka: ports: to 9092:9092
```
- The `docker-compose.yml` file may need to be changed; the ports for kafka should be 9092:9092.
- Startup the kafka and zookeeper containers and make sure they are running.

```bash
$ docker-compose up --no-recreate -d
$ docker-compose ps
```
- **When you want to stop kafka and zookeeper, execute the following commands.**

```bash
$ cd $BASE_PPM_DIR/kafka-docker
$ docker-compose down
```

## 10. Download and install the Kafka **binary**.

-  The Kafka binary provides a producer and consumer tool that can act as surrogates for the ODE (among other items).
-  [Kafka Binary](https://kafka.apache.org/downloads)
-  [Kafka Quickstart](https://kafka.apache.org/quickstart) is a very useful reference.
-  Move and unpack the Kafka code as follows:

```bash
$ cd $BASE_PPM_DIR
$ wget http://apache.claz.org/kafka/0.10.2.1/kafka_2.12-0.10.2.1.tgz   // mirror and kafka version may change; check website.
$ tar -xzf kafka_2.12-0.10.2.1.tgz			               // the kafka version may be different.
$ mv kafka_2.12-0.10.2.1 kafka
```

## 11. Download and install [`librdkafka`](https://github.com/edenhill/librdkafka), the C++ Kafka library we use to build the PPM.

```bash
$ cd $BASE_PPM_DIR
$ git clone https://github.com/edenhill/librdkafka.git
$ cd librdkafka
$ ./configure
$ make
$ sudo make install
```

- **NOTE**: The header files for `librdkafka` should be located in `/usr/local/include/librdkafka` and the libraries
  (static and dynamic) should be located in `/usr/local/lib`. If you put them in another location the PPM may not build.

## 12. Download, Build, and Install the Privacy Protection Module (PPM)

```bash
$ cd $BASE_PPM_DIR
$ git clone https://github.com/usdot-jpo-ode/jpo-cvdp.git
$ cd jpo-cvdp
$ mkdir build && cd build
$ cmake ..
$ make
```

## Additional information

- The PPM uses [RapidJSON](https://github.com/miloyip/rapidjson), but it is a header-only library included in the repository.
- The PPM uses [spdlog](https://github.com/gabime/spdlog) for logging; it is a header-only library and the headers are included in the repository.
- The PPM uses [Catch](https://github.com/philsquared/Catch) for unit testing, but it is a header-only library included in the repository.

# Integrating with the ODE

## Using the Docker Container

This will run the PPM module in separate container. First set the required environmental variables. You need to tell the PPM container where the Kafka Docker container is running with the `DOCKER_HOST_IP` variable. Also tell the PPM container where to find the [map file](configuration.md#map-file) and [PPM Configuration file](configuration.md) by setting the `DOCKER_SHARED_VOLUME`:

```bash
$ export DOCKER_HOST_IP=your.docker.host.ip
$ export DOCKER_SHARED_VOLUME=/your/shared/directory
```

Note that the map file and configuration file must be located in the `DOCKER_SHARED_VOLUME` root directory and named
`config.properties` and `road_file.csv` respectively. 

Add the following service to the end of the `docker-compose.yml` file in the `jpo-ode` installation directory.

```bash
  ppm:
    build: /path/to/jpo-cvdp/repo
    environment:
      DOCKER_HOST_IP: ${DOCKER_HOST_IP}
    volumes:
      - ${DOCKER_SHARED_VOLUME}:/ppm_data
```

Start the ODE containers as normal. Note that the topics for raw BSMs must be created ahead of time.

