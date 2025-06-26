#!/bin/bash
# filepath: /workspace/scripts/build.sh

set -e  # Exit on any error

echo "🔨 Building Thread-Safe Queue Project..."
echo "========================================"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default build type
BUILD_TYPE=${1:-debug}

echo -e "${BLUE}Build type: ${BUILD_TYPE}${NC}"

# Create build directory
echo -e "${YELLOW}Creating build directory...${NC}"
mkdir -p build
cd build

# Configure with CMake or use Make directly
if [ -f "../CMakeLists.txt" ]; then
    echo -e "${YELLOW}Configuring with CMake...${NC}"
    cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE^} ..
    echo -e "${YELLOW}Building with CMake...${NC}"
    cmake --build . --parallel $(nproc)
else
    echo -e "${YELLOW}Building with Make...${NC}"
    cd ..
    make clean
    case ${BUILD_TYPE} in
        "debug")
            make debug
            ;;
        "release")
            make release
            ;;
        "profile")
            make profile
            ;;
        *)
            make all
            ;;
    esac
fi

echo -e "${GREEN}✅ Build completed successfully!${NC}"

# Show built binaries
echo -e "${BLUE}Built binaries:${NC}"
if [ -f "queue_test" ]; then
    ls -la queue_test*
elif [ -f "../build/queue_test" ]; then
    ls -la ../build/queue_test*
fi

echo -e "${GREEN}🎉 Ready to run tests!${NC}"
echo "Use: ./scripts/run_tests.sh"
