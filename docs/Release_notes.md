Jpo-cvdp Release Notes
----------------------------

Version 1.5.0, released January 2025
----------------------------------------
### **Summary**
The jpo-cvdp 1.5.0 release includes an enhancement to redact the "asn1" field from BSM messages, ensuring that filtered BSM messages no longer contain the field. Additionally, it updates GitHub Actions workflows with the latest versions of third-party actions from external repositories to eliminate Node.js and other deprecation warnings.

Enhancements in this release:
- [CDOT PR 54](https://github.com/CDOT-CV/jpo-cvdp/pull/54): BSM "asn1" Field Redaction
- [USDOT PR 45](https://github.com/usdot-jpo-ode/jpo-cvdp/pull/45): Update GitHub Actions Third-Party Action Versions

Known Issues:
- No known issues at this time.


Version 1.4.0, released September 2024
----------------------------------------
### **Summary**
The changes for the jpo-cvdp 1.4.0 release involve the addition of a 'Supported Message Types' section to the README, the removal of TIM support since TIMs do not contain personally-identifiable information, and revisions to the documentation for accuracy & clarity.

Enhancements in this release:
- CDOT PR 44: Added 'Supported Message Types' section to README
- CDOT PR 45: Removed TIM support since TIMs do not contain personally-identifiable information
- CDOT PR 46: Revised documentation for accuracy & clarity


Version 1.3.0, released February 2024
----------------------------------------
### **Summary**
The changes for the jpo-cvdp 1.3.0 release involve the optimization of dockerfiles, addition of dockerhub image documentation & some QoL changes to the `do_kafka_test.sh` script.

Enhancements in this release:
- CDOT PR 32: Optimized dockerfiles for smaller & faster deployments by switching over to using alpine as the base image.
- CDOT PR 37: Added dockerhub image documentation
- CDOT PR 38: Updated `do_kafka_test.sh` script to set DOCKER_HOST_IP automatically
- CDOT PR 39: Updated `do_kafka_test.sh` script to resolve the KAFKA_CONTAINER_NAME env var dynamically

Known Issues:
- No known issues at this time.


Version 1.2.0, released November 2023
----------------------------------------

### **Summary**
The changes for the jpo-cvdp 1.2.0 release involve the addition of CI/CD configuration, a new build.sh script, an updated catch dependency, modified sonar properties, adjusted unit tests for accuracy, standardized logging level strings to uppercase and fixes for the `do_kafka_test.sh` script.
- Added CI/CD configuration
- Added `build.sh` script
- Updated catch dependency
- Modified sonar properties.
- Modified a unit test to use reasonable accuracy.
- Standardized logging level strings to uppercase.
- Fixed the `do_kafka_test.sh` script
- Changed default logging level to ‘ERROR’ instead of ‘TRACE’
- Directly accessed PPM_LOG_LEVEL in PpmLogger class instead of using ‘v’ opt string.
- Set PPM_LOG_LEVEL to DEBUG in `standalone.sh` and `standalone_multi.sh` scripts
- Removed `docker-from-docker` feature from .devcontainer spec
- Replaced instances of `chmod 7777` in dockerfiles with `chmod 777`

Known Issues:
-	No known issues at this time.


Version 1.1.0, released July 5th 2023
----------------------------------------

### **Summary**
The jpo-cvdp 1.1.0 new release version brings several enhancements and fixes as follows:

Enhancements in this release:
- Switched to using Confluent’s librdkafka package instead of the default package provided by Ubuntu because the former was more up-to-date.
- Modified class usage diagram to reflect changes to `kafka_consumer.cpp`
- The `kafka_consumer.cpp` file was not being compiled, so we added a new target to the CMakeLists.txt file and made other necessary changes. Additionally, we included comments in the CMakeLists.txt file to improve readability and assist future project contributors.
- Added a section on the `build_and_exec.sh` script to the README.
- Added missing environment variables to docker-compose files.
- Added github actions docker workflows.

Fixes in this release:
- Refactored the `kafka_consumer.cpp` file to align with object-oriented design principles.
- Reviewed and adjusted log levels for each log statement as needed.
- Replaced instances of `std::cout` and `std::cerr` with the logger wherever possible, except for a few specific cases.
- Expanded on environment variable descriptions in README.

Known Issues
- The do_kafka_test.sh script located in the root directory of the project is currently not running successfully.

Version 1.0.0, released Mar 30th 2023
----------------------------------------

### **Summary**
The updates for jpo-cvdp 1.0.0 include Confluent Cloud integration, general redaction functionality, logging modifications and documentation improvements.

Enhancements in this release:
- Added CDOT build script.
- Documentation updates
- Implemented partII redaction and created a new test case.
- Created Troubleshooting.md
- Made dockerfiles use the v1.6.2 release of librdkafka.
- Allowed the project to work with an instance of kafka hosted by Confluent Cloud.
- Added a section on CC integration to the README.
- Added a section on troubleshooting to the README.
- Added max.partition.fetch.bytes property to some config files.
- Added quick script to build and run unit tests.
- Added dev container.
- Implemented recursive redaction method.
- Added a .gitattributes file to force usage of LF end of line sequence.
- Allowed the project to pull in data and config files from an external source. (generalization)
- Added a class usage diagram to the README.
- Added a data flow diagram.
- Expanded the ‘data & config files’ sub-section of the README.
- Split bsmfilter.h and bsmfilter.cpp into 8 files.
- Created the PpmLogger class to serve as an abstraction layer for spdlog.
- Added flags for whether messages should be logged to a file and/or the console.
- Utilized the PpmLogger class instead of standard output wherever possible.
- Introduced environment variables for file & console logging.
- Formalized general redaction functionality to include coreData in addition to partII data.
- Created the RapidjsonRedactor class to assist with general redaction.
- Condensed general redaction code in BSMHandler class into one method.
- Added test cases for general redaction.
- Updated data flow & class usage diagrams.
- Updated data flow diagram README.
- Included general redaction header/implementation files with the input files for doxygen.
- Added links to configuration.md, testing.md and troubleshooting.md.
- Added data flow diagram to the data flow README.
- Simplified logging solution & added log level environment variable to the project.

Fixes in this release:
-	Fixed docker tag.
-	Swapped to using the base dockerfile for CC integration.
-	Removed ‘success’ echo message from build_cdot.sh script since the script can fail due to authentication issues.
-	Swapped to packaged version of cmake for better efficiency.
-	Swapped to packaged version of librdkafka in dockerfiles.
-	Altered group.id in some properties files.
-	Overrode general redaction behavior in the case of some required leaf members, bitstrings & optional objects w/ required members.

Known Issues
- The do_kafka_test.sh script in the root directory of the project does not run successfully at this time.

### ODE Sprint 38

- ODE-771: Fixed reported bug where the PPM exits when connections to Kafka brokers fail.

### ODE Sprint 15

- ODE-369/ORNL-15: Logging
- Updated Identifier Redactor to include random assignment in lieu of fixed assignment.

### ODE Sprint 14

- ORNL-17: USDOT Playbook

### ODE Sprint 13

- ODE-290: Integration with the ODE.

### ODE Sprint 12

- ODE-77: Complete documentation

### ODE Sprint 11

- (Partial Complete) ODE-282 Implement a Module that Interfaces with the ODE.
- (Partially Complete) ODE-77 Implement a PPM that uses a Geofence to Filter BSMs.
