# jpo-cvdp
The United States Department of Transportation Joint Program Office (JPO)
Connected Vehicle Data Privacy (CVDP) project is developing a variety of methods
to enhance the privacy of individuals who generate connected vehicle data.

Connected vehicle technology uses in-vehicle wireless transceivers to broadcast
and receive basic safety messages (BSMs) that include accurate spatiotemporal
information to enhance transportation safety. Integrated Global Positioning
System (GPS) measurements are included in BSMs.  Databases, some publicly
available, of BSM sequences, called trajectories, are being used to develop
safety and traffic management applications. **BSMs do not contain explicit
identifiers that link trajectories to individuals; however, the locations they
expose may be sensitive and associated with a very small subset of the
population; protecting these locations from unwanted disclosure is extremely
important.** Developing procedures that minimize the risk of associating
trajectories with individuals is the objective of this project.

# The Operational Data Environment (ODE) Privacy Protection Module (PPM)
The PPM operates on streams of raw BSMs processed by the ODE. It determines
whether individual BSMs should be retained or suppressed (deleted) based on the
information in that BSM and auxiliary map information used to define a geofence.
BSM geoposition (latitude and longitude) and speed are used to determine the
disposition of each BSM processed. Additionally, the PPM redacts a configurable
set of fields from the BSMs that are retained to further protect privacy.

## PPM Limitations
Protecting against inference-based privacy attacks on spatiotemporal
trajectories (i.e., sequences of BSMs from a single vehicle) in **general** is
a challenging task. An example of an inference-based privacy attack is
identifying the driver that generated a sequence of BSMs using specific
locations they visit during their trip, or other features discernable from the
information in the BSM sequence. **This PPM treats a specific use case: a
geofenced area where residences do not exist, e.g., a highway corridor, with
certain speed restrictions.** Do not assume this strategy will work in general.
There are alternative strategies that must be employed to handle cases where
loitering locations can aid in learning the identity of the driver.

## Supported Message Types
- Basic Safety Message (BSM)

It should be noted that the PPM is not designed to handle other message types at this time. Future versions of the PPM may support additional message types.

## Table of Contents
1. [Release Notes](#release-notes)
2. [Documentation](#documentation)
3. [Development and Collaboration Tools](#development-and-collaboration-tools)
4. [Getting Started](#getting-started)
5. [Confluent Cloud Integration](#confluent-cloud-integration)
6. [Testing/Troubleshooting](#testingtroubleshooting)
7. [General Redaction](#general-redaction)

### Additional Resources
1. [Installation](docs/installation.md)
2. [Configuration and Operation](docs/configuration.md)
3. [Testing](docs/testing.md)
4. [Development](docs/coding-standards.md)

## Release Notes
The current version and release history of the Jpo-cvdp: [Jpo-cvdp Release Notes](<docs/Release_notes.md>)

## Documentation

The following document will help practitioners build, test, deploy, and understand the PPM's functions:

- [Privacy Protection Module User Guide](docs/ppm_user_manual.docx)

All stakeholders are invited to provide input to these documents. Stakeholders should direct all input on this document
to the JPO Product Owner at DOT, FHWA, or JPO. To provide feedback, we recommend that you create an "issue" in this
repository (https://github.com/usdot-jpo-ode/jpo-cvdp/issues). You will need a GitHub account to create an issue. If you
don’t have an account, a dialog will be presented to you to create one at no cost.

### Code Documentation

Code documentation can be generated using [Doxygen](https://www.doxygen.org) by following the commands below:

```bash
$ sudo apt-get update
$ sudo apt-get install doxygen
$ cd <install root>/jpo-cvdp
$ doxygen
```

The documentation is in HTML and is written to the `<install root>/jpo-cvdp/docs/html` directory. Open `index.html` in a
browser.

### Class Usage Diagram
![class usage](./docs/diagrams/class-usage/PPM%20Class%20Usage.drawio.png)

This diagram displays how the different classes in the project are used. If one class uses another class, there will be a black arrow pointing to the class it uses. The Tool class is extended by the PPM class, which is represented by a white arrow.

## Development and Collaboration Tools

### Source Repositories - GitHub

- https://github.com/usdot-jpo-ode/jpo-cvdp
- `git@github.com:usdot-jpo-ode/jpo-cvdp.git`

## Getting Started

### Prerequisites

You will need Git to obtain the code and documents in this repository.
Furthermore, we recommend using Docker to build the necessary containers to
build, test, and experiment with the PPM.

- [Git](https://git-scm.com/)
- [Docker](https://www.docker.com)

You can find more information in our [installation and setup](docs/installation.md) directions.

### Getting the Source Code

See the installation and setup instructions unless you just want to examine the code.

**Step 1.** Disable Git `core.autocrlf` (Only the First Time)

   **NOTE**: If running on Windows, please make sure that your global git config is
   set up to not convert End-of-Line characters during checkout. This is important
   for building docker images correctly.

```bash
git config --global core.autocrlf false
```

**Step 2.** Clone the source code from GitHub repositories using Git commands:

```bash
git clone https://github.com/usdot-jpo-ode/jpo-cvdp.git
```

## Confluent Cloud Integration
Rather than using a local kafka instance, this project can utilize an instance of kafka hosted by Confluent Cloud via SASL.

### Environment variables
#### Purpose & Usage
- The DOCKER_HOST_IP environment variable is used to communicate with the bootstrap server that the instance of Kafka is running on.
- The KAFKA_TYPE environment variable specifies what type of kafka connection will be attempted and is used to check if Confluent should be utilized. If this is not set to "CONFLUENT", the PPM will attempt to connect to a local kafka instance.
- The CONFLUENT_KEY and CONFLUENT_SECRET environment variables are used to authenticate with the bootstrap server. These are the API key and secret that are generated when a new API key is created in Confluent Cloud. These are only used if the KAFKA_TYPE environment variable is set to "CONFLUENT".

#### Values
- DOCKER_HOST_IP must be set to the bootstrap server address (excluding the port)
- KAFKA_TYPE must be set to "CONFLUENT"
- CONFLUENT_KEY must be set to the API key being utilized for CC
- CONFLUENT_SECRET must be set to the API secret being utilized for CC

### CC Docker Compose File
There is a provided docker-compose file (docker-compose-confluent-cloud.yml) that passes the above environment variables into the container that gets created. Further, this file doesn't spin up a local kafka instance since it is not required.

### Note
This has only been tested with Confluent Cloud but technically all SASL authenticated Kafka brokers can be reached using this method.

## Testing/Troubleshooting
### Unit Tests
Unit tests can be built and executed using the build_and_run_unit_tests.sh file inside of the dev container for the project. Alternatively, they can be run inside of the deployed PPM container. More information about this can be found [here](./docs/testing.md#unit-testing).

### Standalone Cluster
The docker-compose.yml file is meant for local testing/troubleshooting.

To utilize this, run the following command in the root directory of the project:
> docker compose up

Sometimes kafka will fail to start up properly. If this happens, spin down the containers and try again.

#### Data & Config Files
Data and config files are expected to be in a location pointed to by the DOCKER_SHARED_VOLUME environment variable.

At this time, the PPM assumes that this location is the /ppm_data directory. When run in a docker or k8s solution, an external drive/directory can be mounted to this directory.

In a BSM configuration, the PPM requires the following files to be present in the /ppm_data directory:
- *.edges
- ppmBsm.properties

##### fieldsToRedact.txt
The path to this file is specified by the REDACTION_PROPERTIES_PATH environment variable. If this is not set, field redaction will not take place but the PPM will continue to function. If this is set and the file is not found, the same behavior will occur.

When running the project in the provided dev container, the REDACTION_PROPERTIES_PATH environment variable should be set to the project-level fieldsToRedact.txt file for debugging/experimentation purposes. This is located in /workspaces/jpo-cvdp/config/fieldsToRedact.txt from the perspective of the dev container.

##### RPM Debug
If the RPM_DEBUG environment variable is set to true, debug messages will be logged to a file by the RedactionPropertiesManager class. This will allow developers to see whether the environment variable is set, whether the file was found and whether a non-zero number of redaction fields were loaded in.

### Build & Exec Script
The [`build_and_exec.sh`](./build_and_exec.sh) script can be used to build a tagged image of the PPM, run the container & enter it with an interactive shell. This script can be used to test the PPM in a standalone environment.

This script should be run outside of the dev container in an environment where Docker is available.

It should be noted that this script needs to use the LF end-of-line sequence.

### Kafka Test Script
The [do_kafka_test.sh](./do_kafka_test.sh) script is designed to perform integration tests on a Kafka instance. To execute the tests, this script relies on the `standalone.sh` and `do_bsm_test.sh` scripts.

To ensure proper execution, it is recommended to run this script outside of the dev container where docker is available. This is because the script will spin up a standalone kafka instance and will not be able to access the docker daemon from within the dev container.

It should be noted that this script and any dependent scripts need to use the LF end-of-line sequence. These include the following:
- do_kafka_test.sh
- standalone.sh
- do_bsm_test.sh
- test_in.py
- test_out.py

The DOCKER_HOST_IP environment variable must be set to the IP address of the host machine. This is required for the script to function properly. This can be set by using the following command:

```
export DOCKER_HOST_IP=$(ifconfig | zgrep -m 1 -oP '(?<=inet\s)\d+(\.\d+){3}')
```

WSL will sometimes hang while the script waits for kafka to create topics. The script should exit after a number of attempts, but if it does not, running `wsl --shutdown` in a windows command prompt and restarting the docker services is recommended.

### Some Notes
- The tests for this project can be run after compilation by running the "ppm_tests" executable. An easy way to do this is to run the `build_and_exec.sh` script and then run the executable from within the container, which should be located in the /cvdi-stream-build directory.
- When manually compiling with WSL, librdkafka will sometimes not be recognized. This can be avoided by utilizing the provided dev environment.

## General Redaction
General redaction refers to redaction functionality in the BSMHandler that utilizes the 'fieldsToRedact.txt' file to redact specified fields from BSM messages.

### How to specify the fields to redact
The fieldsToRedact.txt file is used by the BSMHandler and lists the paths to the fields to be redacted. It should be noted that this file needs to use the LF end-of-line sequence.

#### How are fields redacted?
The paths in the fieldsToRedact.txt file area are added to a list and then used to search for the fields in the BSM message. If a member is found, the default behavior is to remove it with rapidjson's RemoveMember() function. It should be noted that by default, only leaf members are able to be removed. There are some exceptions to this which are listed in the [Overridden Redaction Behavior](#overridden-redaction-behavior) section.

### Overridden Redaction Behavior
Some values will be treated differently than others when redacted. For example, the 'coreData.angle' field will be set to 127 instead of being removed since it is a required field. The following table lists the overridden redaction behavior.

| Field | Redaction Behavior |
| --- | --- |
| angle | Set to 127 |
| transmission | Set to "UNAVAILABLE" |
| wheelBrakes | Set first bit to 1 and all other bits to 0 |
| weatherProbe | Remove object |
| status | Remove object |
| speedProfile | Remove object |
| traction | Set to "unavailable" |
| abs | Set to "unavailable" |
| scs | Set to "unavailable" |
| brakeBoost | Set to "unavailable" |
| auxBrakes | Set to "unavailable" |
