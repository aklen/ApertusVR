# Setting up ApertusVR on Debian Linux

## Step 1: Update System and Install Dependencies
Run the following commands to update your system and install the required dependencies:

```bash
sudo apt-get update -y && sudo apt-get upgrade -y
sudo apt install -y \
    build-essential \
    cmake \
    make \
    git \
    curl \
    libcurl4-openssl-dev \
    libc6-dev \
    linux-libc-dev \
    gcc-10 \
    g++-10
```

GStreamer (optional)
```bash
sudo apt install -y libgstreamer1.0-dev gstreamer1.0-plugins-base gstreamer1.0-tools gstreamer1.0-plugins-good gstreamer1.0-plugins-bad
```

Ensure the correct compiler versions are used:

```bash
export CC=/usr/bin/gcc-10
export CXX=/usr/bin/g++-10
```

## Step 2: Clone the ApertusVR Repository
Clone the specific branch needed for the project:

```bash
git clone --branch AHA/audio --single-branch https://github.com/aklen/ApertusVR.git
cd ApertusVR
```

## Step 3: Build ApertusVR
Run the build script:

```bash
./build_unix.sh
```

## Step 4: Running ApertusVR
After a successful build, navigate to the `bin` directory and run a sample project:

```bash
cd build/bin
./apeSampleLauncher ../samples/helloWorld/
```

## Notes
- Ensure all dependencies are installed correctly before building.
- If encountering issues with missing libraries, check dependencies using:

  ```bash
  ldd ./apeSampleLauncher
  ```
  
- If additional modules are required, they can be enabled in the `CMakeLists.txt` configuration before building.

With this setup, ApertusVR should be successfully compiled and running on your Raspberry Pi. 🚀
