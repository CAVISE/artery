# Distribution tag
ARG TAG=base   # or 'latest' (Arch is rolling)

FROM archlinux:${TAG} AS setup

SHELL ["/bin/bash", "-c"]

RUN pacman -Syu --noconfirm \
      base-devel cmake git python python-pip \
      bison flex wget curl \
      libxml2 zlib boost crypto++ fox gdal proj geographiclib xerces-c \
      ninja pkgconf zeromq protobuf \
      clang clang-tools-extra \
  && pacman -Scc --noconfirm

FROM setup AS build

# SUMO version (github tag)
ARG SUMO_TAG=v1_21_0
# OMNeT version (github tag)
ARG OMNETPP_TAG=omnetpp-5.6.2

RUN git clone --recurse --depth 1 --branch ${OMNETPP_TAG} https://github.com/omnetpp/omnetpp
WORKDIR /omnetpp
RUN mv configure.user.dist configure.user
RUN source setenv -f                                            \
    && ./configure WITH_QTENV=no WITH_OSG=no WITH_OSGEARTH=no   \
    && make -j$(nproc --all) base MODE=release

WORKDIR /
RUN git clone --recurse --depth 1 --branch ${SUMO_TAG} https://github.com/eclipse-sumo/sumo
WORKDIR /sumo
RUN cmake -B build .                                    \
        -G Ninja                                        \
        -DCMAKE_BUILD_CONFIG=Release                    \
        -DCMAKE_INSTALL_PREFIX=/sumo-prefix             \
        -DENABLE_CS_BINDINGS=OFF                        \
        -DENABLE_JAVA_BINDINGS=OFF                      \
        -DENABLE_PYTHON_BINDINGS=OFF                    \
        -DNETEDIT=OFF                                   \
    && cmake --build build --parallel $(nproc --all)    \
    && cmake --install build

FROM setup AS final

COPY --from=build /omnetpp/bin /omnetpp/bin
COPY --from=build /omnetpp/include /omnetpp/include
COPY --from=build /omnetpp/lib /omnetpp/lib
COPY --from=build /omnetpp/images /omnetpp/images
COPY --from=build /omnetpp/Makefile.inc /omnetpp

COPY --from=build /sumo-prefix/ /usr/local

RUN cd /usr/local/bin && \
    curl -sSL -O https://raw.githubusercontent.com/llvm/llvm-project/main/clang-tools-extra/clang-tidy/tool/clang-tidy-diff.py && \
    chmod +x clang-tidy-diff.py

ENV PATH=/omnetpp/bin:$PATH
ENV SUMO_HOME=/usr/local/share/sumo
