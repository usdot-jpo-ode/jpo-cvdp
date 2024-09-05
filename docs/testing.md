# Testing the PPM
This document describes how to test the PPM module. The PPM can be tested using unit tests, standalone tests, and Kafka integration tests.

## Table of Contents
- [Unit Testing](#unit-testing)
- [Standalone Testing](#standalone-testing)
- [Kafka Integration Testing](#kafka-integration-testing)
- [Test Files](#test-files)
- [See Also: Testing/Troubleshooting](#see-also-testingtroubleshooting)

## Unit Testing
### Testing On Local Machine
The build_and_run_unit_test.sh script provides an easy method to build and run the PPM's unit tests. It should be noted that this script needs to have the LF end-of-line sequence for it to work.

#### Steps
1. Pull the project into VSCode
1. Reopen the project in a dev container
1. Open the terminal.
1. Type "sudo su" to run commands as root
1. Type ./build_and_run_unit_tests.sh to run the script

### Testing Using Docker
1. Start by building the Docker image:

```bash
$ docker build -t ppm .
```

2. Then run unit tests inside the container with the following command:

```bash
$ docker run -it -e PPM_LOG_TO_CONSOLE=true --name ppm ppm /cvdi-stream-build/ppm_tests
```

3. Remove the container:

```bash
$ docker rm ppm
```

## Standalone Testing
1. Spin up Kafka & the PPM
```
$ docker compose up -d --build
```

2. View logs of PPM
```
$ docker compose logs -f ppm
```

3. Listen to the output topic
```
$ kafkacat -b localhost:9092 -t topic.OdeBsmJson -C
```

4. Send a message to the PPM using kafkacat
```
$ kafkacat -b localhost:9092 -t topic.OdeBsmJson -P
```

You can now paste a JSON message into the terminal and hit enter. The PPM should log the message and send it to the output topic if it is not suppressed.

The message immediately to the right of `BSM` indicates whether the message was RETAINED, or passed on to a filtered stream, or SUPPRESSED with the cause. The information in parenthesis is the TemporaryID, secMark, lat, lon, and speed information in the message; this can be used to test and troubleshoot your configuration.

```bash
[170613 12:30:47.057503] [info] BSM [RETAINED]: (ON-VG---,36710,41.116496,-104.888494,5.000000)
[170613 12:30:47.057566] [info] BSM [RETAINED]: (ON-VG-99,36711,41.116496,-104.888494,5.000000)
[170613 12:30:47.057584] [info] BSM [SUPPRESSED-speed]: (ON-VBL--,36712,41.116496,-104.888494,1.000000)
[170613 12:30:47.057593] [info] BSM [SUPPRESSED-speed]: (ON-VBH--,36713,41.116496,-104.888494,100.000000)
[170613 12:30:47.057623] [info] BSM [RETAINED]: (OFFVG---,36714,41.118110,-104.889282,5.000000)
[170613 12:30:47.057639] [info] BSM [SUPPRESSED-speed]: (OFFVBH--,36715,41.118110,-104.889282,99.000000)
[170613 12:30:47.057670] [info] BSM [RETAINED]: (OFFVGMID,36716,41.141742,-105.361760,9.000000)
[170613 12:30:47.057705] [info] BSM [RETAINED]: (ON-VGTOP,36717,41.143138,-105.361470,9.000000)
[170613 12:30:47.058086] [info] BSM [SUPPRESSED-speed]: (ON-VBTOP,36718,41.143138,-105.361470,1.000000)
[170613 12:30:47.058126] [info] BSM [RETAINED]: (ON-VGBOT,36719,41.140537,-105.362255,9.000000)
[170613 12:30:47.058147] [info] BSM [SUPPRESSED-speed]: (ON-VBBOT,36720,41.140537,-105.362255,50.000000)
[170613 12:30:47.058178] [info] BSM [RETAINED]: (ON-VG---,36721,41.411728,-110.137350,9.000000)
[170613 12:30:47.058213] [info] BSM [RETAINED]: (ON-VG-99,36722,41.411728,-110.137350,9.000000)
[170613 12:30:47.058293] [info] BSM [RETAINED]: (OFFVG---,36723,41.628687,-109.089771,9.000000)
[170613 12:30:47.058451] [info] BSM [RETAINED]: (OFFVG---,36724,41.627758,-109.091004,9.000000)
[170613 12:30:47.058494] [info] BSM [RETAINED]: (O??VG---,36725,41.627672,-109.089390,9.000000)
[170613 12:30:47.064880] [info] BSM [RETAINED]: (ON-VG---,36726,41.627467,-109.089251,9.000000)
[170613 12:30:47.064940] [info] BSM [RETAINED]: (OFFVG---,36727,43.313653,-111.799675,9.000000)
```

5. To stop the PPM and Kafka, run the following command:
```
$ docker compose down
```

## Kafka Integration Testing
To run kafka integration tests, run the following command:
```bash
$ ./do_kafka_test.sh
```

## Test Files

Several example JSON message test files are in the [jpo-cvdp/data](../data) directory.  These files can be edited to generate
your own test cases. Each line in the file should be a well-formed BSM JSON
object. **Each message should be on a separate line in the file.** **If a JSON object cannot be parsed it is suppressed.**

# See Also: Testing/Troubleshooting
More information on testing can be found in the [Testing/Troubleshooting](../README.md#testingtroubleshooting) section of the README.