FROM ubuntu:18.04 as builddependencies
RUN apt-get update -y \
    && apt-get install -y --no-install-recommends software-properties-common \
    && add-apt-repository ppa:ubuntu-toolchain-r/test \
    && apt-get update -y \
    && apt install -y --no-install-recommends \
        software-properties-common \
        build-essential \
        make \
        cmake \
        curl \
        libcurl4 \
        libcurl4-openssl-dev \
        vim \
        git \
        libc6-dev \
        linux-libc-dev \
        gcc-10 \
        g++-10 \
    && apt-get clean
RUN export CC=/usr/bin/gcc-10 && export CXX=/usr/bin/g++-10

FROM builddependencies as gitclone
WORKDIR /home
RUN git clone -b core-commands --single-branch https://github.com/aklen/ApertusVR.git

FROM gitclone as cmakeprepare
RUN mkdir ApertusVR-build
WORKDIR /home/ApertusVR-build
RUN cmake -DCMAKE_BUILD_TYPE="Release" -DCMAKE_CONFIGURATION_TYPES="Release" -B ./ -S ../ApertusVR

FROM cmakeprepare as build
RUN make

FROM build as release
WORKDIR /home/ApertusVR-build/bin
RUN ./apeSampleLauncher ../../ApertusVR/samples/helloWorld/
