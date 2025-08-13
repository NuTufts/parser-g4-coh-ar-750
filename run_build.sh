#!/bin/bash

# Build script for g4-coh-ar-750 parsers

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building g4-coh-ar-750 parsers...${NC}"

# Check if cenns is built - try both build and builddir
if [ ! -v CENNS10_INSTALL_DIR ]; then
    echo "The environment variable 'CENNS10_INSTALL_DIR' has not been defined."
    echo "You must build the g4-coh-ar-750 sim and setup it's environment variables for this project to properly setup the build environment."
    return
else
    echo "Found g4-coh-ar-750 install dir at: ${CENNS10_INSTALL_DIR}"
fi

if [ ! -d "$CENNS10_BUILD_DIR" ]; then
    if [ ! -d "$CENNS10_BUILD_DIR/lib/cmake/cenns" ]; then
        echo -e "${RED}Error: cenns package not found in build director:${NC}"
        echo -e "${YELLOW}Please build and install the main cenns simulation first:${NC}"
        echo "  cd [to g4-coh-ar-750 repo]"
        echo "  source setenv.sh"
        echo "  cd ${CENNS10_BUILD_DIR}"
        echo "  cmake .."
        echo "  make install"
        return
    fi
fi

echo -e "${GREEN}Found cenns installation in: $CENNS10_INSTALL_DIR${NC}"

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure with CMake
# Point to the cenns installation
echo -e "${GREEN}Configuring with CMake...${NC}"
cmake -Dcenns_DIR=$CENNS10_INSTALL_DIR/lib/cmake/cenns ..

# Check if CMake succeeded
if [ $? -ne 0 ]; then
    echo -e "${RED}CMake configuration failed!${NC}"
    exit 1
fi

# Build
echo -e "${GREEN}Building...${NC}"
make -j$(nproc)

# Check if build succeeded
if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build successful!${NC}"
    echo -e "${GREEN}Executable created: build/bin/flatten_edep_info${NC}"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi