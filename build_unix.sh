#!/bin/bash

set -e  # Stop script on error

echo "🔧 Setting up the build environment..."

# Define source and build paths
APE_SOURCE_PATH="${APE_SOURCE_PATH:-$(pwd)}"
APE_BUILD_PATH="${1:-${APE_BUILD_PATH:-$APE_SOURCE_PATH/build}}"

# Set flags
BUILD_TYPE="Release"

# Compiler settings
export CC=/usr/bin/gcc-10
export CXX=/usr/bin/g++-10

# Cleanup previous build
echo "🧹 Cleaning up old build files..."
# rm -rf "$APE_BUILD_PATH"
# rm -rf "$APE_BUILD_PATH"/CMakeCache.txt
# rm -rf "$APE_BUILD_PATH"/CMakeFiles/
mkdir -p "$APE_BUILD_PATH"
cd "$APE_BUILD_PATH"

# Run CMake with proper flags
echo "⚙️ Running CMake..."
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DUSE_LIBCURL=ON \
      -G "Unix Makefiles" \
      "$APE_SOURCE_PATH"

# Build the project
echo "🚀 Building the project..."
make -j$(nproc)

echo "✅ Build completed successfully!"
