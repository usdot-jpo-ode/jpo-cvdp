# Installation and Setup
## Table of Contents
1. [Docker Installation](#docker-installation)
1. [Manual Installation](#manual-installation)
1. [CDOT Integration with K8s](#cdot-integration-with-k8s)

## Docker Installation
### 1. Install [Docker](https://www.docker.com)

- When following the website instructions, setup the Docker repos and follow the Linux post-install instructions.
- The CE version seems to work fine.
- [Docker installation instructions](https://docs.docker.com/engine/installation/linux/ubuntu/#install-using-the-repository)

#### ORNL Specific Docker Configuration
- *ORNL specific, but may apply to others with organizational security*
    - Correct for internal Google DNS blocking
    - As root (`$ sudo su`), create a `daemon.json` file in the `/etc/docker` directory that contains the following information:
```bash
          {
              "debug": true,
              "default-runtime": "runc",
              "dns": ["160.91.126.23","160.91.126.28"],
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

Be sure to restart the docker daemon to consume the new configuration file.

```bash
$ service docker stop
$ service docker start
```

Check the configuration using the command below to confirm the updates above are taken if needed:

```bash
$ docker info
```

### 2. Configure environment variables
Configure the environment variables for the PPM to communicate with the Kafka instance. Copy or rename the `sample.env` file to `.env`.

```bash
$ cp sample.env .env
```

Edit the `.env` file to include the necessary information.

```bash
$ vi .env
```

For more information on the environment variables, see the 'Environment Variables' section in the [configuration.md](configuration.md) file.

### 3. Spin up Kafka & the PPM in Docker
To spin up the PPM and Kafka in Docker, use the following commands:

```bash
docker compose up --build
```

## Manual Installation
The following instructions represent the "hard" way to install and test the PPM. A docker image can be built to make
this easier: [Using the Docker Container](#using-the-docker-container). *The directions that follow were developed for a clean installation of Ubuntu.*

### 1. Install [Git](https://git-scm.com/)

```bash
$ sudo apt install git
```

### 2. Install librdkafka

Talking to a Kafka instance, subscribing and producting to topics requires the use of a third party library. We use the librdkafka library as a c/c++ implementation. This can be installed as a package.

```bash
$ sudo apt install -y libsasl2-dev 
$ sudo apt install -y libsasl2-modules
$ sudo apt install -y libssl-dev 
$ sudo apt install -y librdkafka-dev
```

### 3. Install [CMake](https://cmake.org), g++ & make to build the PPM

```bash
$ sudo apt install cmake
$ sudo apt install g++
$ sudo apt install make
```

### 4. Download, Build, and Install the Privacy Protection Module (PPM)

```bash
$ cd $BASE_PPM_DIR
$ git clone https://github.com/usdot-jpo-ode/jpo-cvdp.git
$ cd jpo-cvdp
$ mkdir build && cd build
$ cmake ..
$ make
```

### Additional information

- The PPM uses [RapidJSON](https://github.com/miloyip/rapidjson), but it is a header-only library included in the repository.
- The PPM uses [spdlog](https://github.com/gabime/spdlog) for logging; it is a header-only library and the headers are included in the repository.
- The PPM uses [Catch](https://github.com/philsquared/Catch) for unit testing, but it is a header-only library included in the repository.

## CDOT Integration with K8s

### Overview
The Colorado Department of Transportation (CDOT) is deploying the various ODE services within a Kubernetes (K8s) environment. Details of this deployment can be found in the main ODE repository [documentation pages](https://github.com/usdot-jpo-ode/jpo-ode/docs). In general, each submodule image is built as a Docker image and then pushed to the CDOT registry. The images are pulled into containers running within the K8s environment, and additional containers are spun up as load requires.

### CDOT PPM Module Build
Several additional files have been added to this project to facilitate the CDOT integration. These files are:
- cdot-scripts/build_cdot.sh
- docker-test/ppm_no_map.sh

#### Shell Scripts
Two additional scripts have been added to facilitate the CDOT integration. The first, [`ppm_no_map.sh`](../docker-test/ppm_no_map.sh), is modeled after the existing [`ppm.sh`](../docker-test/ppm.sh) script and performs a similar function. This script is used to start the PPM module, but leaves out the hard-coded mapfile name in favor of the properties file configuration. The second script, [`build_cdot.sh`](../cdot-scripts/build_cdot.sh), is used to build the CDOT PPM Docker image, tag the image with a user provided tag, and push that image to a remote repository. This is a simple automation script used to help reduce complexity in the CDOT pipeline.