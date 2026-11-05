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
    pkg-config libzmq5-dev cppzmq-dev libprotobuf-dev \
    protobuf-compiler python3-pip               \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --recurse --depth 1 --branch ${OMNETPP_TAG} https://github.com/omnetpp/omnetpp
WORKDIR /omnetpp
RUN mv configure.user.dist configure.user
# Artery runs in one OMNeT++ process; disable distributed simulation and MPI.
RUN source setenv -f                                                                \
    && ./configure WITH_QTENV=no WITH_TKENV=no WITH_OSG=no WITH_OSGEARTH=no WITH_PARSIM=no \
    && make -j$(nproc --all) base MODE=release

WORKDIR /
RUN git clone --recurse --depth 1 --branch ${SUMO_TAG} https://github.com/eclipse-sumo/sumo
WORKDIR /sumo
RUN cmake -B build .                                    \
        -G Ninja                                        \
        -DCMAKE_BUILD_TYPE=Release                      \
        -DCMAKE_INSTALL_PREFIX=/sumo-prefix             \
        -DCMAKE_DISABLE_FIND_PACKAGE_FOX=ON             \
        -DENABLE_CS_BINDINGS=OFF                        \
        -DENABLE_JAVA_BINDINGS=OFF                      \
        -DENABLE_PYTHON_BINDINGS=OFF                    \
        -DNETEDIT=OFF                                   \
    && cmake --build build --parallel $(nproc --all)    \
    && cmake --install build

FROM debian:${TAG} AS ci

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build git curl      \
    python3 python3-pip python3-venv clang-tidy     \
    libxml2-dev zlib1g-dev libboost-all-dev         \
    libcrypto++-dev libgdal-dev libproj-dev         \
    libgeographiclib-dev libxerces-c-dev            \
    pkg-config libzmq5-dev cppzmq-dev libprotobuf-dev \
    protobuf-compiler libgtest-dev                  \
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

# CI installs system dependencies at image build time and runs jobs without sudo.
ARG USER=artery
ARG USER_UID=1001
ARG USER_GID=1001
RUN test "${USER_UID}" -gt 0 && test "${USER_GID}" -gt 0 && \
    groupadd --gid ${USER_GID} ${USER} && \
    useradd --create-home --uid ${USER_UID} --gid ${USER_GID} --shell /bin/bash ${USER} && \
    install -d -o ${USER_UID} -g ${USER_GID} /workspaces/artery /cached-build
ENV HOME=/home/${USER}
USER ${USER}
WORKDIR /workspaces/artery

FROM ci AS artery-build
ARG BUILD_CONFIG=RelWithDebInfo
COPY --chown=${USER_UID}:${USER_GID} . /workspaces/artery/
RUN ./tools/build.py -cb --build-dir /cached-build --generator Ninja --config "${BUILD_CONFIG}"

# Runtime has no compiler, package-install sudo rule, or graphical environment.
FROM debian:${TAG} AS runtime-base
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates python3 \
    libboost-date-time1.74.0 libboost-filesystem1.74.0 libboost-system1.74.0 \
    libboost-program-options1.74.0 libboost-thread1.74.0 libboost-regex1.74.0 \
    libboost-serialization1.74.0 libboost-iostreams1.74.0 \
    libcrypto++8 libxml2 libgdal32 libproj25 libgeographiclib23 libxerces-c3.2 \
    libprotobuf32 libzmq5 libdw1 libpython3.11 zlib1g \
    && rm -rf /var/lib/apt/lists/*
ARG USER=artery
ARG USER_UID=1000
ARG USER_GID=1000
RUN test "${USER_UID}" -gt 0 && test "${USER_GID}" -gt 0 && \
    groupadd --gid ${USER_GID} ${USER} && \
    useradd --create-home --uid ${USER_UID} --gid ${USER_GID} --shell /bin/bash ${USER} && \
    install -d -o ${USER_UID} -g ${USER_GID} /workspaces/artery /cached-build
ENV HOME=/home/${USER}
ENV PATH=/omnetpp/bin:$PATH
ENV SUMO_HOME=/usr/local/share/sumo
USER ${USER}
WORKDIR /workspaces/artery

FROM runtime-base AS runtime
COPY --from=build /omnetpp/bin /omnetpp/bin
COPY --from=build /omnetpp/lib /omnetpp/lib
COPY --from=build /sumo-prefix/ /usr/local/
COPY --from=artery-build --chown=${USER_UID}:${USER_GID} /workspaces/artery/ /workspaces/artery/
COPY --from=artery-build --chown=${USER_UID}:${USER_GID} /cached-build/ /cached-build/
ARG BUILD_CONFIG=RelWithDebInfo
ENV ARTERY_BUILD_DIR=/cached-build/${BUILD_CONFIG}
# Catch missing runtime libraries when the complete image is built.
RUN python3 - <<'CHECK_LIBRARIES'
from pathlib import Path
import subprocess

missing = []
for root in ("/cached-build", "/omnetpp/lib", "/omnetpp/bin", "/usr/local/bin", "/usr/local/lib"):
    for path in Path(root).rglob("*"):
        if not path.is_file():
            continue
        with path.open("rb") as binary:
            if binary.read(4) != b"\x7fELF":
                continue
        result = subprocess.run(["ldd", str(path)], text=True, capture_output=True)
        for line in result.stdout.splitlines():
            if "not found" in line:
                missing.append(f"{path}: {line.strip()}")
if missing:
    raise SystemExit("Missing runtime libraries:\n" + "\n".join(missing))
CHECK_LIBRARIES
RUN opp_run -h >/dev/null && sumo --version >/dev/null
CMD ["sleep", "infinity"]

# Keep the historical default target for CI consumers that omit --target.
FROM ci AS final
