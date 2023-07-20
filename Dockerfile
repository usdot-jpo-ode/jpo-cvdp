FROM alpine:3.12
USER root

WORKDIR /cvdi-stream

ENV DEBIAN_FRONTEND=noninteractive

# update the package manager
RUN apk update

# add build tools
RUN apk add --upgrade --no-cache --virtual .build-deps \
    build-base \
    cmake \
    git \
    wget \
    gcc \
    g++ \
    linux-headers \
    make \
    musl-dev \
    openssl-dev \
    protobuf-dev \
    protobuf-c-dev \
    sudo \
    bash

# add librdkafka from edge branch
RUN sed -i -e 's/v3\.4/edge/g' /etc/apk/repositories \
    && apk upgrade --update-cache --available \
    && apk add --no-cache librdkafka librdkafka-dev

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