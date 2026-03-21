# Artery's Docker container does not include any GUI.
# The idea of having Artery in a container is to run multiple instances with different parameter sets, e.g. running a large parameter study on a cluster.
# You may want to use Vagrant for a setup with GUI instead.

# Distribution tag
ARG TAG=bookworm-slim

FROM debian:${TAG} AS build

# SUMO version (github tag)
ARG SUMO_TAG=v1_21_0
# OMNeT version (github tag)
ARG OMNETPP_TAG=omnetpp-5.6.2

SHELL [ "/bin/bash", "-c"]
RUN apt-get update && apt-get install -y        \
    bison build-essential flex git python3-dev  \
    libxml2-dev wget zlib1g-dev cmake            \
    libboost-all-dev libcrypto++-dev            \
    libfox-1.6-dev libgdal-dev libproj-dev      \
    libgeographiclib-dev libxerces-c-dev        \
    ninja-build curl python3-venv clang-tidy    \
    pkg-config libzmq5-dev libprotobuf-dev      \
    protobuf-compiler python3-pip               \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --recurse --depth 1 --branch ${OMNETPP_TAG} https://github.com/omnetpp/omnetpp
WORKDIR /omnetpp
RUN mv configure.user.dist configure.user
RUN source setenv -f                                                                \
    && ./configure WITH_QTENV=no WITH_TKENV=no WITH_OSG=no WITH_OSGEARTH=no WITH_MPI=no \
    && make -j$(nproc --all) base MODE=release

WORKDIR /
RUN git clone --recurse --depth 1 --branch ${SUMO_TAG} https://github.com/eclipse-sumo/sumo
WORKDIR /sumo
RUN cmake -B build .                                    \
        -G Ninja                                        \
        -DCMAKE_BUILD_CONFIG=Release                    \
        -DCMAKE_INSTALL_PREFIX=/sumo-prefix             \
        -DENABLE_GUI=OFF                                \
        -DENABLE_CS_BINDINGS=OFF                        \
        -DENABLE_JAVA_BINDINGS=OFF                      \
        -DENABLE_PYTHON_BINDINGS=OFF                    \
        -DNETEDIT=OFF                                   \
    && cmake --build build --parallel $(nproc --all)    \
    && cmake --install build

FROM debian:${TAG} AS final

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build git curl      \
    python3 python3-pip python3-venv clang-tidy     \
    libxml2-dev zlib1g-dev libboost-all-dev         \
    libcrypto++-dev libgdal-dev libproj-dev         \
    libgeographiclib-dev libxerces-c-dev            \
    pkg-config libzmq5-dev libprotobuf-dev          \
    protobuf-compiler                               \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /omnetpp/bin /omnetpp/bin
COPY --from=build /omnetpp/include /omnetpp/include
COPY --from=build /omnetpp/lib /omnetpp/lib
COPY --from=build /omnetpp/images /omnetpp/images
COPY --from=build /omnetpp/Makefile.inc /omnetpp
COPY --from=build /sumo-prefix/ /usr/local

RUN git clone --depth 1 --branch v0.23.0 https://github.com/ZedThree/clang-tidy-review.git /tmp/clang-tidy-review && \
    python3 -m pip install --no-cache-dir --break-system-packages /tmp/clang-tidy-review/post/clang_tidy_review && \
    rm -rf /tmp/clang-tidy-review

ENV PATH=/omnetpp/bin:$PATH
ENV SUMO_HOME=/usr/local/share/sumo