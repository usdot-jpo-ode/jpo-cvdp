# Installation Setup and Testing

The following instructions represent the "hard" way to install and test the PPM. A docker image can be built to make
this easier (see X). *The directions that follow were developed for a clean installation of Ubuntu.*

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

## 10. Create a base directory from which to install all the necessary components to test the PPM.

```bash
$ export BASE_PPM_DIR=~/some/dir/you/want/to/put/this/stuff
```

## 11. Install [`kafka-docker`](https://github.com/wurstmeister/kafka-docker) so kafka and zookeeper can run in a separate container.

- These services will need to be running to test the PPM.

```bash
$ cd $BASE_PPM_DIR
$ git clone https://github.com/wurstmeister/kafka-docker.git
$ ifconfig                                              // to get the host ip address (device ens33 maybe)
$ export DOCKER_HOST_IP=<HOST IP>
$ cd kafka-docker
$ vim docker-compose.yml	                        // Set karka: ports: to 9092:9092
$ docker-compose up --no-recreate -d                    // to startup kafka and zookeeper containers and not recreate
$ docker-compose ps                                     // to check that they are running.
```

## 12. When you want to stop kafka and zookeeper

```bash
$ cd $BASE_PPM_DIR/kafka-docker
$ docker-compose down
```

## 13. Download and install the Kafka **binary**.

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

## 14. Download and install [`librdkafka`](https://github.com/edenhill/librdkafka), the C++ Kafka library we use to build the PPM.

- Upon completion of the instructions below, the header files for `librdkafka` should be located in `/usr/local/include/librdkafka`
  and the libraries (static and dynamic) should be located in `/usr/local/lib`. If you put them in another location
  the PPM may not build.

```bash
$ cd $BASE_PPM_DIR
$ git clone https://github.com/edenhill/librdkafka.git
$ cd librdkafka
$ ./configure
$ make
$ sudo make install
```

## 15. Download, build, and install the Privacy Protection Module (PPM)

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
