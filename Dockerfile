# === BUILDER IMAGE ===
FROM alpine:3.12 as builder
USER root

WORKDIR /cvdi-stream

ENV DEBIAN_FRONTEND=noninteractive

# update the package manager
RUN apk update

# add build dependencies
RUN apk add --upgrade --no-cache --virtual .build-deps \
    cmake \
    g++ \
    make

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

# do the build
RUN export LD_LIBRARY_PATH=/usr/local/lib && mkdir /cvdi-stream-build && cd /cvdi-stream-build && cmake /cvdi-stream && make

# === RUNTIME IMAGE ===
FROM alpine:3.12
USER root

WORKDIR /cvdi-stream

# add runtime dependencies
RUN apk add --upgrade --no-cache \
    bash

# add librdkafka from edge branch
RUN sed -i -e 's/v3\.4/edge/g' /etc/apk/repositories \
    && apk upgrade --update-cache --available \
    && apk add --no-cache librdkafka librdkafka-dev

# copy the built files from the builder
COPY --from=builder /cvdi-stream-build/ /cvdi-stream-build/
COPY --from=builder /cvdi-stream /cvdi-stream

# Add test data. This changes frequently so keep it low in the file.
ADD ./docker-test /cvdi-stream/docker-test

# Run the tool.
RUN chmod 7777 /cvdi-stream/docker-test/ppm_no_map.sh
CMD ["/cvdi-stream/docker-test/ppm_no_map.sh"]