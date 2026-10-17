#!/bin/bash
# DongArch3D - Run All Tests (Linux/macOS)
# Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute

set -e  # Exit on error

echo "=========================================="
echo "DongArch3D v4.0 - Test Runner"
echo "=========================================="

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Find project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "Project Root: $PROJECT_ROOT"

# Check build directory
BUILD_DIR="$PROJECT_ROOT/GigaMesh/build"

if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}Error: Build directory not found${NC}"
    echo "Please run CMake first:"
    echo "  mkdir -p GigaMesh/build && cd GigaMesh/build"
    echo "  cmake .. -DCMAKE_BUILD_TYPE=Release"
    echo "  make -j8"
    exit 1
fi

cd "$BUILD_DIR"

# Step 1: Build tests
echo ""
echo -e "${YELLOW}Step 1: Building tests...${NC}"
make -j8 test_dongarch

if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Build successful${NC}"

# Step 2: Run CTest
echo ""
echo -e "${YELLOW}Step 2: Running CTest...${NC}"
ctest --output-on-failure --verbose

if [ $? -ne 0 ]; then
    echo -e "${RED}Tests failed!${NC}"
    exit 1
fi

echo -e "${GREEN}✓ All tests passed${NC}"

# Step 3: Optional - Run Valgrind (Linux only)
if [ "$(uname)" = "Linux" ]; then
    echo ""
    echo -e "${YELLOW}Step 3: Running Valgrind memory check...${NC}"

    if command -v valgrind &> /dev/null; then
        valgrind --leak-check=full --error-exitcode=1 ./gui/tests/test_dongarch

        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓ No memory leaks detected${NC}"
        else
            echo -e "${RED}✗ Memory leaks detected${NC}"
        fi
    else
        echo -e "${YELLOW}⚠ Valgrind not installed, skipping memory check${NC}"
    fi
fi

# Summary
echo ""
echo "=========================================="
echo -e "${GREEN}All tests completed successfully!${NC}"
echo "=========================================="
