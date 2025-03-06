#!/bin/bash

set -e  # Stop script on error

echo "🔧 Setting up the build environment..."

# Define source and build paths
APE_SOURCE_PATH="${APE_SOURCE_PATH:-$(pwd)}"  # Default to current directory if not set
APE_BUILD_PATH="${1:-${APE_BUILD_PATH:-$APE_SOURCE_PATH/build}}"  # Use first argument, env variable, or default

# Detect macOS SDK path
SDK_PATH=$(xcrun --sdk macosx --show-sdk-path)

# Set compiler flags
CXX_FLAGS="-isysroot $SDK_PATH -stdlib=libc++"
BUILD_TYPE="Release"

# Optional: Use Homebrew Clang if installed
if command -v brew &> /dev/null && brew list llvm &> /dev/null; then
    echo "🍺 Using Homebrew Clang..."
    export CC=/opt/homebrew/opt/llvm/bin/clang
    export CXX=/opt/homebrew/opt/llvm/bin/clang++
fi

# Custom env setup (ha szükséges)
export CMAKE_SYSTEM_PREFIX_PATH="$SDK_PATH/usr"
export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"

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
      -DCMAKE_CXX_FLAGS="$CXX_FLAGS" \
      -DCMAKE_OSX_SYSROOT=$SDK_PATH \
      -DUSE_LIBCURL=ON \
      -G "Unix Makefiles" \
      "$APE_SOURCE_PATH"

# Build the project
echo "🚀 Building the project..."
make -j$(sysctl -n hw.ncpu)

echo "✅ Build completed successfully!"
