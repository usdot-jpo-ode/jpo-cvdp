Jpo-cvdp Release Notes
----------------------------

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


