#!/bin/bash
# Build script for RTSP Camera Viewer on Ubuntu 22.04

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== RTSP Camera Viewer Build Script (Ubuntu 22.04) ===${NC}"
echo ""

# Check if dependencies are installed
echo -e "${YELLOW}Checking dependencies...${NC}"

missing_deps=()

check_command() {
    if ! command -v "$1" &> /dev/null; then
        missing_deps+=("$1")
    fi
}

check_pkg_config() {
    if ! pkg-config --exists "$1" 2>/dev/null; then
        missing_deps+=("$2")
    fi
}

check_command cmake
check_command g++
check_command make
check_command pkg-config

check_pkg_config "sdl2" "libsdl2-dev"
check_pkg_config "SDL2_ttf" "libsdl2-ttf-dev"
check_pkg_config "libavcodec" "libavcodec-dev"
check_pkg_config "libavformat" "libavformat-dev"
check_pkg_config "libswscale" "libswscale-dev"
check_pkg_config "libswresample" "libswresample-dev"

if [ ! -f "/usr/include/spdlog/spdlog.h" ]; then
    missing_deps+=("libspdlog-dev")
fi

if [ ! -f "/usr/include/nlohmann/json.hpp" ]; then
    missing_deps+=("nlohmann-json3-dev")
fi

if [ ! -f "/usr/include/concurrentqueue/concurrentqueue.h" ]; then
    missing_deps+=("libconcurrentqueue-dev")
fi

if [ ${#missing_deps[@]} -ne 0 ]; then
    echo -e "${RED}Missing dependencies:${NC}"
    for dep in "${missing_deps[@]}"; do
        echo "  - $dep"
    done
    echo ""
    echo -e "${YELLOW}Install them with:${NC}"
    echo "sudo apt update && sudo apt install -y ${missing_deps[*]}"
    exit 1
fi

echo -e "${GREEN}✓ All dependencies found${NC}"
echo ""

# Create build directory
echo -e "${YELLOW}Creating build directory...${NC}"
mkdir -p build
cd build

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake -DCMAKE_BUILD_TYPE=Release ..

echo ""
echo -e "${YELLOW}Building project...${NC}"
make -j$(nproc)

echo ""
echo -e "${GREEN}✓ Build complete!${NC}"
echo ""
echo -e "Executable: ${GREEN}build/RTSPCameraViewer${NC}"
echo ""
echo "Run with: ./build/RTSPCameraViewer"
