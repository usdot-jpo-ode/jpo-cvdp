FROM ubuntu:20.04
USER root

WORKDIR /cvdi-stream

ENV DEBIAN_FRONTEND=noninteractive

# Add build tools.
RUN apt-get update && apt-get install -y software-properties-common wget git make gcc-7 g++-7 gcc-7-base && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 100 && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 100

# Install cmake.
RUN apt install -y libprotobuf-dev protobuf-compiler
RUN apt install -y cmake

# Install librdkafka.
RUN apt-get install -y sudo
RUN wget -qO - https://packages.confluent.io/deb/7.3/archive.key | sudo apt-key add -
RUN add-apt-repository "deb [arch=amd64] https://packages.confluent.io/deb/7.3 stable main"
RUN add-apt-repository "deb https://packages.confluent.io/clients/deb $(lsb_release -cs) main"
RUN apt update
RUN apt-get install -y libsasl2-modules libsasl2-modules-gssapi-mit libsasl2-dev libssl-dev 
RUN apt install -y librdkafka-dev

# add the source and build files
ADD CMakeLists.txt /cvdi-stream
ADD ./src /cvdi-stream/src
ADD ./cv-lib /cvdi-stream/cv-lib
ADD ./include /cvdi-stream/include
ADD ./kafka-test /cvdi-stream/kafka-test
ADD ./unit-test-data /cvdi-stream/unit-test-data
ADD ./config /cvdi-stream/config

# Do the build.
RUN export LD_LIBRARY_PATH=/usr/local/lib && mkdir /cvdi-stream-build && cd /cvdi-stream-build && cmake /cvdi-stream && make

# Add test data. This changes frequently so keep it low in the file.
ADD ./docker-test /cvdi-stream/docker-test

# Run the tool.
RUN chmod 7777 /cvdi-stream/docker-test/ppm_no_map.sh
CMD ["/cvdi-stream/docker-test/ppm_no_map.sh"]