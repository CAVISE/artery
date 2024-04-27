FROM ubuntu:18.04

ARG SUMO=true
ARG VERSION=5.6.2
ARG REPOSITORY='https://github.com/CAVISE/artery.git'
ARG BRANCH='DRL_FL'

RUN mkdir -p /app/Cavise
WORKDIR /app/Cavise
ENV DEBIAN_FRONTEND=noninteractive
# Install software-properties-common and pip3
RUN apt-get update && apt-get install -y software-properties-common \
    build-essential \
    cmake \
    libproj-dev \
    libxerces-c-dev \
    python3 \
    python3-setuptools \
    wget \
    python3-pip \
    git \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install pyzmq

# Download and install specific version of CMake
RUN apt-get update && \
    apt-get install -y software-properties-common lsb-release && \
    apt-get clean all && \
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null && \
    apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" && \
    apt-get update && \
    apt-get install -y kitware-archive-keyring && \
    rm /etc/apt/trusted.gpg.d/kitware.gpg && \
    apt-get install -y cmake && \
    cmake --version

########################################################
# Install SUMO
########################################################
# Add the SUMO PPA and install SUMO
RUN if [ true = true ] ; then \
    add-apt-repository ppa:sumo/stable \
    && apt-get update \
    && apt-get install -y sumo sumo-tools sumo-doc \
    && pip3 install traci \
    ; else \
    echo "Installation without SUMO" ; fi

ENV SUMO_HOME=/usr/share/sumo

########################################################
# Install OMNeT++
########################################################
# Requires several packages to be installed on the computer. 
# These packages include the C++ compiler (gcc or clang), 
# the Java runtime, and several other libraries and programs.
RUN apt-get install -y build-essential gcc g++ bison flex perl \
    python python3 qt5-default \
    libqt5opengl5-dev \
    tcl-dev \
    tk-dev \
    libxml2-dev zlib1g-dev default-jre doxygen graphviz libwebkitgtk-3.0-0 \
    && apt-get install -y openscenegraph-plugin-osgearth libosgearth-dev \
    && apt-get install -y openmpi-bin libopenmpi-dev \
    && apt-get install -y libpcap-dev \
    && apt-get install -y libprotobuf-dev protobuf-compiler \
    && apt-get install -y libzmq3-dev

# Clone Omnet repository
RUN wget https://github.com/omnetpp/omnetpp/releases/download/omnetpp-$VERSION/omnetpp-$VERSION-src-core.tgz \
    --progress=bar:force:noscroll -O omnetpp-src-core.tgz && \
    tar xf omnetpp-src-core.tgz && \
    rm omnetpp-src-core.tgz && \
    mv omnetpp-$VERSION /omnetpp
WORKDIR /omnetpp
ENV PATH /omnetpp/bin:$PATH
RUN ./configure WITH_QTENV=no WITH_OSG=no WITH_OSGEARTH=no && \
    make -j $(nproc) base MODE=release && \
    make -j $(nproc) base MODE=debug

########################################################
# Install Artery 
########################################################

RUN apt install -y libgeographic-dev libcrypto++-dev \
    && apt install -y libboost-dev libboost-date-time-dev libboost-system-dev

WORKDIR /app/Cavise
RUN git clone --recurse-submodule $REPOSITORY --branch $BRANCH --single-branch
WORKDIR /app/Cavise/artery
RUN mkdir -p /app/Cavise/artery/build \
    && pwd \
    && cd /app/Cavise/artery/build \
    && cmake .. \
    && cmake --build .

