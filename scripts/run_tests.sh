#!/bin/bash
# filepath: /workspace/scripts/run_tests.sh

set -e

echo "🧪 Running Thread-Safe Queue Tests..."
echo "====================================="

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Test configuration
TEST_BINARY="queue_test"
BUILD_DIR="build"
OUTPUT_DIR="output"

# Create output directory
mkdir -p ${OUTPUT_DIR}

# Find test binary
if [ -f "${BUILD_DIR}/${TEST_BINARY}" ]; then
    TEST_PATH="${BUILD_DIR}/${TEST_BINARY}"
elif [ -f "${TEST_BINARY}" ]; then
    TEST_PATH="./${TEST_BINARY}"
else
    echo -e "${RED}❌ Test binary not found. Run build.sh first.${NC}"
    exit 1
fi

echo -e "${BLUE}Found test binary: ${TEST_PATH}${NC}"

# Function to run a test with timeout
run_test() {
    local test_name="$1"
    local timeout_duration="$2"
    local output_file="${OUTPUT_DIR}/${test_name}.txt"
    
    echo -e "${YELLOW}Running ${test_name}...${NC}"
    
    if timeout ${timeout_duration} ${TEST_PATH} > ${output_file} 2>&1; then
        echo -e "${GREEN}✅ ${test_name} completed successfully${NC}"
        echo "Output saved to: ${output_file}"
    else
        local exit_code=$?
        if [ ${exit_code} -eq 124 ]; then
            echo -e "${YELLOW}⏰ ${test_name} timed out after ${timeout_duration} seconds${NC}"
        else
            echo -e "${RED}❌ ${test_name} failed with exit code ${exit_code}${NC}"
        fi
        echo "Output saved to: ${output_file}"
    fi
    
    echo "----------------------------------------"
}

# Run different test scenarios
echo -e "${BLUE}🚀 Starting test suite...${NC}"

# Basic functionality test
run_test "basic_functionality" "30s"

# Multi-threading stress test
echo -e "${YELLOW}Running multi-threading stress test...${NC}"
if timeout 60s ${TEST_PATH} --stress > ${OUTPUT_DIR}/stress_test.txt 2>&1; then
    echo -e "${GREEN}✅ Stress test completed${NC}"
else
    echo -e "${YELLOW}⏰ Stress test timed out or failed${NC}"
fi

# Performance benchmark
echo -e "${YELLOW}Running performance benchmark...${NC}"
if timeout 45s ${TEST_PATH} --benchmark > ${OUTPUT_DIR}/benchmark.txt 2>&1; then
    echo -e "${GREEN}✅ Benchmark completed${NC}"
else
    echo -e "${YELLOW}⏰ Benchmark timed out${NC}"
fi

# Memory leak test with Valgrind (if available)
if command -v valgrind &> /dev/null; then
    echo -e "${YELLOW}Running memory leak detection...${NC}"
    if timeout 120s valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
        ${TEST_PATH} > ${OUTPUT_DIR}/valgrind_memcheck.txt 2>&1; then
        echo -e "${GREEN}✅ Memory leak test completed${NC}"
    else
        echo -e "${YELLOW}⏰ Valgrind test timed out${NC}"
    fi
fi

# Race condition detection with Helgrind (if available)
if command -v valgrind &> /dev/null; then
    echo -e "${YELLOW}Running race condition detection...${NC}"
    if timeout 180s valgrind --tool=helgrind --history-level=approx \
        ${TEST_PATH} > ${OUTPUT_DIR}/helgrind_analysis.txt 2>&1; then
        echo -e "${GREEN}✅ Race condition test completed${NC}"
    else
        echo -e "${YELLOW}⏰ Helgrind test timed out${NC}"
    fi
fi

# Summary
echo -e "${BLUE}📊 Test Summary${NC}"
echo "==============="
echo "Test outputs available in: ${OUTPUT_DIR}/"
ls -la ${OUTPUT_DIR}/

# Check for common issues
echo -e "${YELLOW}🔍 Quick Analysis:${NC}"

if [ -f "${OUTPUT_DIR}/valgrind_memcheck.txt" ]; then
    if grep -q "ERROR SUMMARY: 0 errors" ${OUTPUT_DIR}/valgrind_memcheck.txt; then
        echo -e "${GREEN}✅ No memory leaks detected${NC}"
    else
        echo -e "${RED}⚠️  Memory issues detected - check valgrind_memcheck.txt${NC}"
    fi
fi

if [ -f "${OUTPUT_DIR}/helgrind_analysis.txt" ]; then
    if grep -q "ERROR SUMMARY: 0 errors" ${OUTPUT_DIR}/helgrind_analysis.txt; then
        echo -e "${GREEN}✅ No race conditions detected${NC}"
    else
        echo -e "${RED}⚠️  Race conditions detected - check helgrind_analysis.txt${NC}"
    fi
fi

echo -e "${GREEN}🎉 Test suite completed!${NC}"
