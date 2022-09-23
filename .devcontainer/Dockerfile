# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.224.2/containers/cpp/.devcontainer/base.Dockerfile

# [Choice] Debian / Ubuntu version (use Debian 11, Ubuntu 18.04/21.04 on local arm64/Apple Silicon): debian-11, debian-10, ubuntu-21.04, ubuntu-20.04, ubuntu-18.04
ARG VARIANT="bullseye"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

# [Optional] Install CMake version different from what base image has already installed. 
# CMake reinstall choices: none, 3.21.5, 3.22.2, or versions from https://cmake.org/download/
ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="none"

# Optionally install the cmake for vcpkg
COPY ./reinstall-cmake.sh /tmp/
RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# [Optional] Uncomment this section to install additional vcpkg ports.
# RUN su vscode -c "${VCPKG_ROOT}/vcpkg install <your-port-name-here>"

# [Optional] Uncomment this section to install additional packages.
# RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#     && apt-get -y install --no-install-recommends <your-package-list-here>

<<<<<<< HEAD
# install librdkafka
RUN apt-get install -y libsasl2-dev libsasl2-modules libssl-dev librdkafka-dev

ENV REDACTION_PROPERTIES_PATH /workspaces/jpo-cvdp/config/fieldsToRedact.txt
ENV RPM_DEBUG true
=======
RUN apt-get update
RUN apt-get install -y libsasl2-dev libsasl2-modules libssl-dev librdkafka-dev

ENV REDACTION_PROPERTIES_PATH /workspaces/jpo-cvdp/config/fieldsToRedact.txt
ENV RPM_DEBUG true
>>>>>>> d59b1876159aa5269ec7ca443554fde0674f6714
