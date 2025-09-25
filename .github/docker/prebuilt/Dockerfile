FROM ghcr.io/cavise/artery:base AS prebuilt

SHELL ["/bin/bash", "-c"]

WORKDIR /workspace/artery

# Which CMake configs to prebuild; space-separated, e.g. "Debug Release"
ARG BUILD_CONFIGS="Debug"

COPY . .

RUN for cfg in ${BUILD_CONFIGS}; do                 \
      echo "building config: $cfg";                 \
      ./tools/build.py -cb                          \
        --build-dir /opt/build                      \
        --generator Ninja                           \
        --config "$cfg";                            \
    done

RUN mkdir -p /opt/build-image && mv build/ /opt/build-cache/
