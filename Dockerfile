# INSTRUCTION
# sudo docker build --no-cache --build-arg ARG ... -t artery:develop .
# xhost +local:docker  
# sudo docker run --privileged --gpus all --network=host -e DISPLAY=$DISPLAY -v /usr/share/vulkan/icd.d:/usr/share/vulkan/icd.d -v path/to/your/artery:/Cavise/artery -it artery:develop /bin/bash
FROM debian:bullseye

# add sumo to deps
ARG SUMO=true
# OMNeT release tag
ARG TAG=omnet-5.6.2
# use remote (to be used in pipelines)
ARG REMOTE=false
# repo for remote
ARG REPOSITORY='https://github.com/CAVISE/artery.git'
# branch to build from
ARG BRANCH='master'

WORKDIR /cavise
ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt upgrade -y
RUN apt install -y software-properties-common build-essential cmake \
    python3 python3-pip wget python3-venv bison git
RUN apt install -y debhelper xdg-user-dirs xserver-xorg libvulkan1 libsdl2-2.0-0 \
    libsm6 libgl1-mesa-glx libomp5 unzip libtiff5 nano fontconfig

RUN ln -s /usr/bin/python3 /bin/python3
RUN pip3 install conan

########################################################
# Install SUMO
########################################################
RUN if [ "${SUMO}" = "true" ]; then                     \
        apt install -y sumo sumo-tools sumo-doc         \
    ; else                                              \
        echo "Installation without SUMO"                \
    ; fi
ENV SUMO_HOME=/usr/share/sumo

RUN rm -r /var/lib/apt/lists/*
RUN if [ "${REMOTE}" = "true" ]; then                                                   \
        git clone --recurse-submodules $REPOSITORY --branch $BRANCH --single-branch &&  \
        git submodule init && git submodule update                                      \
    ; fi

WORKDIR /cavise/artery
SHELL [ "/bin/bash", "-c" ]
ADD . /cavise/artery/
RUN source tools/setup/configure.sh && ./tools/setup/build.py -c -b --config Debug

CMD "echo \"run this interactively\""
