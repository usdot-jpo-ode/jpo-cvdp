FROM ubuntu:18.04
USER root

WORKDIR /cvdi-stream

# Add build tools.
RUN apt-get update && apt-get install -y software-properties-common wget git make gcc-7 g++-7 gcc-7-base && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 100 && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 100

# Install cmake.
RUN apt install -y libprotobuf-dev protobuf-compiler
RUN apt install -y cmake

# Install librdkafka.
RUN apt-get install -y unzip && wget https://github.com/edenhill/librdkafka/archive/refs/tags/v1.6.2.zip && unzip v1.6.2.zip && cd librdkafka-1.6.2 && ln -s /usr/bin/python3 /usr/bin/python && ./configure && make && make install && cd /cvdi-stream

# add the source and build files
ADD CMakeLists.txt /cvdi-stream
ADD ./src /cvdi-stream/src
ADD ./cv-lib /cvdi-stream/cv-lib
ADD ./include /cvdi-stream/include
ADD ./kafka-test /cvdi-stream/kafka-test
ADD ./unit-test-data /cvdi-stream/unit-test-data

# Do the build.
RUN export LD_LIBRARY_PATH=/usr/local/lib && mkdir /cvdi-stream-build && cd /cvdi-stream-build && cmake /cvdi-stream && make

# Add test data. This changes frequently so keep it low in the file.
ADD ./docker-test /cvdi-stream/docker-test

# Run the tool.
CMD ["/cvdi-stream/docker-test/ppm.sh"]
