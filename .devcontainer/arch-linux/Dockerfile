# Distribution tag
ARG TAG=base-devel

FROM archlinux:${TAG} AS setup

SHELL [ "/bin/bash", "-c"]

RUN pacman -Syu --noconfirm     \
    --disable-download-timeout  \
    cmake python3 python-pip    \
    xerces-c fox gdal proj      \
    gl2ps jre17-openjdk swig    \
    maven eigen flex bison git  \
    gcc sdl2 libsm openmp       \
    openscenegraph qt5-base     \
    pacman-contrib sudo         \
    boost boost-libs crypto++ xorg \
    ttf-ubuntu-font-family      \
    ninja cppzmq protobuf       \
    pkg-config                  \
    && pip install --break-system-packages conan \
    && paccache -r -k 0 \
    && find /usr/lib/cmake -name "*.cmake" \
       | xargs -r grep -l "libboost" \
       | xargs -r sed -i -E '/IMPORTED_LOCATION/s|(libboost[^"]*\.so)\.[0-9]+\.[0-9]+\.[0-9]+|\1|g' \
    && for so in $(find /usr/lib/cmake -name "*.cmake" \
           | xargs -r grep -ohP '/usr/lib/libboost_\w+\.so' \
           | sort -u); do \
         if [ ! -f "$so" ]; then \
           printf 'void f(){}' | gcc -x c - -shared -fPIC \
             -Wl,-soname,"$(basename "$so")" -o "$so"; \
         fi; \
       done

###############
# Build stage #
###############

FROM setup AS build

# SUMO version (github tag)
ARG SUMO_TAG=v1_26_0
# OMNeT version (github tag)
ARG OMNETPP_TAG=omnetpp-5.6.2
# GeographicLib version (github tag)
ARG GEOGRAPHICLIB_TAG=v2.5

RUN git clone --recurse --depth 1 --branch ${OMNETPP_TAG} https://github.com/omnetpp/omnetpp \
    && cd omnetpp \
    && mv configure.user.dist configure.user \
    && source setenv -f && ./configure WITH_OSGEARTH=no && make -j$(nproc --all) \
    && rm -rf .git

RUN git clone --recurse --depth 1 --branch ${SUMO_TAG} https://github.com/eclipse-sumo/sumo \
    && cd sumo \
    && (rm -rf /usr/lib/cmake/boost_process* 2>/dev/null; true) \
    && cmake -B build . \
        -G Ninja \
        -DENABLE_JAVA_BINDINGS=OFF \
        -DENABLE_CS_BINDINGS=OFF \
        -DENABLE_PYTHON_BINDINGS=OFF \
    && cmake --build build --parallel $(nproc --all) \
    && cmake --install build --prefix /opt/inst \
    && rm -rf /opt/inst/share/man \
    && cd .. && rm -rf sumo

RUN git clone --recurse --depth 1 --branch ${GEOGRAPHICLIB_TAG} https://github.com/geographiclib/geographiclib \
    && cd geographiclib \
    && cmake -B build . && cmake --build build --parallel $(nproc --all) \
    && cmake --install build --prefix /opt/inst \
    && rm -rf /opt/inst/share/man \
    && cd .. && rm -rf geographiclib

######################
# Installation stage #
######################

FROM setup AS final

# User name for this container
ARG USER=devcontainer

COPY --from=build /omnetpp/bin /omnetpp/bin
COPY --from=build /omnetpp/include /omnetpp/include
COPY --from=build /omnetpp/lib /omnetpp/lib
COPY --from=build /omnetpp/images /omnetpp/images
COPY --from=build /omnetpp/Makefile.inc /omnetpp

COPY --from=build /opt/inst/ /usr/local/

RUN groupadd sudo && useradd -m -G sudo ${USER}
RUN echo "${USER} ALL=(ALL:ALL) NOPASSWD: ALL" | sudo tee /etc/sudoers.d/${USER}

USER ${USER}

ENV PATH=/omnetpp/bin:$PATH
ENV SUMO_HOME=/usr/local/share/sumo

FROM final AS cached-build

ARG USER=devcontainer

COPY --from=final /omnetpp /omnetpp
COPY --from=final /usr/local /usr/local

USER root
RUN mkdir workspace && chown ${USER} workspace 

USER ${USER}
WORKDIR /workspace/artery

# Which CMake configs to prebuild; space-separated, e.g. "Debug Release"
ARG BUILD_CONFIGS="RelWithDebInfo"

COPY --chown=${USER} . .

RUN for cfg in ${BUILD_CONFIGS}; do                 \
      echo "building config: $cfg";                 \
      ./tools/build.py -cb                          \
        --build-dir /workspace/cached-build         \
        --generator Ninja                           \
        --config "$cfg";                            \
    done
