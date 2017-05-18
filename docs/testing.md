# Testing the PPM

There are several ways to test the capabilities of the PPM.

- Testing as an individual application within [Docker](#docker-testing)
- Testing as an individual application on the [native client](#testing-on-the-native-client)
- [Unit Testing](#unit-testing)

## Test Files

Several example JSON BSM test files are in the [jpo-cvdp/data](../data) directory.  These files can be edited to generate
your own test cases. Each line in the file should be a well-formed BSM JSON
object. **Each BSM should be on a separate line in the file.** **If a JSON object cannot be parsed it is suppressed.**

## Docker Testing

To run a series of configuration tests:

    $ ./do_test.sh

**Running this command for the first time can take awhile, as the dependencies need to be built for the PPM image.**
To run the standalone PPM test using Docker containers, from the jpo-cvdp root directory:

    $ ./start_kafka.sh

This will build and start the required kafka containers, including the PPM image.
Next run:

    $ ./test-scripts/standalone.sh [MAP_FILE] [CONFIG] [TEST_FILE] [OFFSET]

Where MAP_FILE is a [map file](configuration.md#map-file), CONFIG is [PPM Configuration file](configuration.md) and TEST_FILE is a [JSON BSM test file](#test-files). Offset refers to the offset in the filtered BSM topic; this is where the consumer will look for new output. The default offset is zero (the beginning of the topic), which should for the first time running the test. This will start the PPM Kafka container and use the supplied files to test the BSM filtering. For example, running:

    $ ./test-scripts/standalone.sh data/I_80.edges config/test/I_80_vel_filter.properties data/I_80_test.json

yields:

```
**************************
Running standalone test with data/I_80.edges config/test/I_80_vel_filter.properties data/I_80_test.json
**************************
**************************
Producing Raw BSMs...
**************************
Producing BSM with ID=BEA10000, speed=7.02, position=41.738136, -106.587029
Producing BSM with ID=BEA10000, speed=7.12, position=41.608656, -109.226824
Producing BSM with ID=BEA10000, speed=7.16, position=41.311097, -110.512927
Producing BSM with ID=BEA10000, speed=7.44, position=41.246647, -111.027436
Producing BSM with ID=BEA10000, speed=7.44, position=41.600371, -106.22341
Producing BSM with ID=BEA10000, speed=1.78, position=42.29789, -83.72035
Producing BSM with ID=BEA10000, speed=0.7, position=42.29789, -83.72034
Producing BSM with ID=BEA10000, speed=6.86, position=42.24576, -83.62337
Producing BSM with ID=BEA10000, speed=6.84, position=42.24576, -83.62337
Producing BSM with ID=BEA10000, speed=6.74, position=42.24576, -83.62338
**************************
Consuming Filtered BSMs at offset 0 ...
**************************
Consuming BSM with ID=BEA10000, speed=7.02, position=41.738136, -106.587029
Consuming BSM with ID=BEA10000, speed=7.12, position=41.608656, -109.226824
Consuming BSM with ID=BEA10000, speed=7.16, position=41.311097, -110.512927
Consuming BSM with ID=BEA10000, speed=7.44, position=41.246647, -111.027436
Consuming BSM with ID=BEA10000, speed=7.44, position=41.600371, -106.22341
Consuming BSM with ID=BEA10000, speed=6.86, position=42.24576, -83.62337
Consuming BSM with ID=BEA10000, speed=6.84, position=42.24576, -83.62337
Consuming BSM with ID=BEA10000, speed=6.74, position=42.24576, -83.62338
```

## Testing on the Native Client

The PPM can be tested as a component of the ODE, and it can be tested using a basic Kafka installation on the native
client.

### ODE Integration Testing

The PPM is meant to be a module that supports the ODE.  The following instructions outline how to perform integration
testing on a single linux installation:

1. Follow the [ODE installation instructions](https://github.com/usdot-jpo-ode/jpo-ode#documentation)
1. Start a terminal for launching the ODE containers.
1. Set the following environment variables:

```bash
$ export DOCKER_HOST_IP=<your.host.ip>
$ export DOCKER_SHARED_VOLUME=<your.shared.directory>
```

1. Follow the Deploying ODE Application on a Docker Host directions

```bash
$ docker-compose up --no-recreate -d
```

1. Start a terminal for launching the PPM.

```bash
$ cd $BASE_PPM_DIR/jpo-cvdp/build
$ ./ppm -c ../config/<testconfig>.properties
```
1. Open a web browser, and enter the url: `localhost:8080`
1. Click on the **Connect** button.
1. Click on the **Browse** button, find a JSON test file with BSMs (one per line).
1. Click the **Upload** button.

The BSMs from the file should be listed in the web browser: the BSMs section
lists all the BSMs; the Filtered BSMs section contains the BSMs that were
processed by the PPM and returned back to the ODE.

### Testing without the ODE

These instructions describe how to run a collection of BSM test JSON objects through the PPM and examine its operation.
Using *GNU screen* for this work is really handy; you will need several shells.

Startup `kafka-docker` in its own shell:

```bash
$ cd $BASE_PPM_DIR/kafka-docker
$ docker-compose up --no-recreate -d                    // to startup kafka and zookeeper containers
$ docker-compose ps                                     // to check that they are running.
```

In another shell, create the simulated ODE produced topic (`j2735BsmRawJson`) and PPM produced topic (`j2735BsmFilteredJson`)

```bash
$ cd $BASE_PPM_DIR/kafka
$ bin/kafka-topic.sh --create --zookeeper <HOST IP>:2181 --replication-factor 1 --partitions 1 --topic j2735BsmRawJson
$ bin/kafka-topic.sh --create --zookeeper <HOST IP>:2181 --replication-factor 1 --partitions 1 --topic j2735BsmFilteredJson
```

Startup the simulated ODE consumer in the same shell you used to create the topics.

```bash
$ cd $BASE_PPM_DIR/kafka                                
$ bin/kafka-console-consumer.sh --bootstrap-server <HOST IP>:9092 --topic j2735BsmFilteredJson
```

- This process should just wait for input from the PPM module.

In another shell, startup the PPM

```bash
$ cd $BASE_PPM_DIR/jpo-cvdp/build
$ ./ppm -c ../config/<testconfig>.properties
```

- At this point the PPM will wait for streaming messages from the simulated ODE
  producer.  When a message is received output will be generated describing how
  the PPM handled the BSM.

In another shell, send test JSON-encoded BSMs to the PPM.

```bash
$ cd $BASE_PPM_DIR/kafka                                
$ cat $BASE_PPM_DIR/jpo-cvdp/data/<testfile> | bin/kafka-console-producer.sh --broker-list <HOST IP>:9092 --topic j2735BsmRawJson
```

- After the messages are written to the `j2735BsmRawJson` topic the shell process should return.

You can confirm PPM operations in two ways:

- Return to the PPM shell and examine the output; it should behave based on PPM configuration settings and the test BSMs.
- Return to the simulated ODE consumer shell and examine the output JSON; this is more difficult because the JSON does not render well on the screen.

## Testing All Capabilities

- To execute the following tests, you will stop the PPM module, if running, with `<CTRL>-C` and then start it up with one of the following `<testconfig>.properties` files.
    - `test.allon.properties`
    - `test.geofenceonly.properties`
    - `test.idredactonly.properties`
    - `test.spdonly.properties`
- After starting the PPM module, you will use the shell you created above to send the testfile, [jpo-cvdp/data/testing_data.json](data/testing_data.json) to the Kafka producer:

```bash
$ cat $BASE_PPM_DIR/jpo-cvdp/data/testing_data.json | bin/kafka-console-producer.sh --broker-list <HOST IP>:9092 --topic j2735BsmRawJson
```

- In the PPM shell the output should look something like the following:
```bash
$ ./ppm -c ../config/test.allon.properties
>> Created Consumer: rdkafka#consumer-1
>> Created Producer: rdkafka#producer-2
Retaining BSM: Pos: (41.7381, -106.587), Spd: 7.02 Id: FFFFFFFF
Filtering BSM [parse] : Pos: (90, 180), Spd: -1 Id: UNASSIGNED
Filtering BSM [speed] : Pos: (41.6087, -109.227), Spd: 1.12 Id: FFFFFFFF
Filtering BSM [parse] : Pos: (90, 180), Spd: -1 Id: UNASSIGNED
Filtering BSM [speed] : Pos: (41.3111, -110.513), Spd: 97.16 Id: BEA10009
Retaining BSM: Pos: (41.2466, -111.027), Spd: 7.44 Id: BEA10009
Retaining BSM: Pos: (41.6004, -106.223), Spd: 7.44 Id: BEA10004
Filtering BSM [parse] : Pos: (90, 180), Spd: -1 Id: UNASSIGNED
Filtering BSM [geoposition] : Pos: (42.2979, -83.7203), Spd: -1 Id: BEA10004
Filtering BSM [geoposition] : Pos: (42.2979, -83.7203), Spd: -1 Id: FFFFFFFF
Filtering BSM [geoposition] : Pos: (42.2458, -83.6234), Spd: -1 Id: FFFFFFFF
Filtering BSM [geoposition] : Pos: (42.2458, -83.6234), Spd: -1 Id: BEA10005
Filtering BSM [geoposition] : Pos: (42.2458, -83.6234), Spd: -1 Id: FFFFFFFF
```
- The above output will vary depending on which configuration file you use.

## Unit Testing

Unit tests are built when the PPM is compiled during installation. Those tests can be run using the following command:

```bash
$ ./ppm_tests
```
