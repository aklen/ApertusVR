#!/bin/bash

# gcc 10 is required to build Apertus using C++17 features
export CC=/usr/bin/gcc-10
export CXX=/usr/bin/g++-10

# -B is the build folder, -S is the source folder
cmake -DCMAKE_BUILD_TYPE="Release" -DCMAKE_CONFIGURATION_TYPES="Release" -B ./ -S ../ApertusVR

