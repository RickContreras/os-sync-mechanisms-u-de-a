#!/bin/bash
# filepath: /workspace/tools/valgrind_check.sh

set -e

echo "🔍 Valgrind Memory and Race Condition Analysis"
echo "=============================================="

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
BINARY_PATH=${1:-"build/queue_test"}
OUTPUT_DIR="output/valgrind"
TIMEOUT_DURATION="300s"  # 5 minutes

# Create output directory
mkdir -p ${OUTPUT_DIR}

# Check if binary exists
if [ ! -f "${BINARY_PATH}" ]; then
    echo -e "${RED}❌ Binary not found: ${BINARY_PATH}${NC}"
    echo "Usage: $0 [binary_path]"
    exit 1
fi

# Check if Valgrind is available
if ! command -v valgrind &> /dev/null; then
    echo -e "${RED}❌ Valgrind not found. Install with: sudo apt-get install valgrind${NC}"
    exit 1
fi

echo -e "${BLUE}Analyzing binary: ${BINARY_PATH}${NC}"
echo -e "${BLUE}Output directory: ${OUTPUT_DIR}${NC}"

# Function to run Valgrind analysis
run_valgrind_analysis() {
    local tool="$1"
    local description="$2"
    local extra_args="$3"
    local output_file="${OUTPUT_DIR}/${tool}_analysis.txt"
    
    echo -e "${YELLOW}Running ${description}...${NC}"
    echo "Output will be saved to: ${output_file}"
    
    local valgrind_cmd="valgrind --tool=${tool} ${extra_args} ${BINARY_PATH}"
    
    if timeout ${TIMEOUT_DURATION} ${valgrind_cmd} > ${output_file} 2>&1; then
        echo -e "${GREEN}✅ ${description} completed${NC}"
        
        # Quick analysis of results
        if [ "${tool}" = "memcheck" ]; then
            if grep -q "ERROR SUMMARY: 0 errors" ${output_file}; then
                echo -e "${GREEN}  → No memory errors detected${NC}"
            else
                local error_count=$(grep "ERROR SUMMARY:" ${output_file} | awk '{print $3}')
                echo -e "${RED}  → ${error_count} memory errors found${NC}"
            fi
            
            if grep -q "All heap blocks were freed" ${output_file}; then
                echo -e "${GREEN}  → No memory leaks detected${NC}"
            else
                echo -e "${RED}  → Memory leaks detected${NC}"
            fi
        fi
        
        if [ "${tool}" = "helgrind" ]; then
            if grep -q "ERROR SUMMARY: 0 errors" ${output_file}; then
                echo -e "${GREEN}  → No race conditions detected${NC}"
            else
                local error_count=$(grep "ERROR SUMMARY:" ${output_file} | awk '{print $3}')
                echo -e "${RED}  → ${error_count} race condition errors found${NC}"
            fi
        fi
        
    else
        local exit_code=$?
        if [ ${exit_code} -eq 124 ]; then
            echo -e "${YELLOW}⏰ ${description} timed out after ${TIMEOUT_DURATION}${NC}"
        else
            echo -e "${RED}❌ ${description} failed with exit code ${exit_code}${NC}"
        fi
    fi
    
    echo "----------------------------------------"
}

# 1. Memory leak detection
run_valgrind_analysis "memcheck" "Memory Leak Detection" \
    "--leak-check=full --show-leak-kinds=all --track-origins=yes --verbose"

# 2. Race condition detection
run_valgrind_analysis "helgrind" "Race Condition Detection" \
    "--history-level=approx --conflict-cache-size=16777216"

# 3. Cache and branch prediction analysis
run_valgrind_analysis "cachegrind" "Cache Performance Analysis" \
    "--branch-sim=yes"

# 4. Data race detector (alternative to Helgrind)
run_valgrind_analysis "drd" "Data Race Detection (DRD)" \
    "--check-stack-var=yes --segment-merging=no"

# Generate summary report
SUMMARY_FILE="${OUTPUT_DIR}/analysis_summary.txt"
echo "🔍 Valgrind Analysis Summary" > ${SUMMARY_FILE}
echo "============================" >> ${SUMMARY_FILE}
echo "Analysis Date: $(date)" >> ${SUMMARY_FILE}
echo "Binary: ${BINARY_PATH}" >> ${SUMMARY_FILE}
echo "" >> ${SUMMARY_FILE}

# Check each analysis
for tool in memcheck helgrind cachegrind drd; do
    analysis_file="${OUTPUT_DIR}/${tool}_analysis.txt"
    if [ -f "${analysis_file}" ]; then
        echo "=== ${tool^^} ANALYSIS ===" >> ${SUMMARY_FILE}
        if grep -q "ERROR SUMMARY:" ${analysis_file}; then
            grep "ERROR SUMMARY:" ${analysis_file} >> ${SUMMARY_FILE}
        fi
        echo "" >> ${SUMMARY_FILE}
    fi
done

echo -e "${BLUE}📊 Analysis Summary:${NC}"
cat ${SUMMARY_FILE}

echo -e "${GREEN}🎉 Valgrind analysis completed!${NC}"
echo -e "${BLUE}Detailed reports available in: ${OUTPUT_DIR}/${NC}"
ls -la ${OUTPUT_DIR}/

# Open summary in default editor if available
if [ -n "$EDITOR" ]; then
    echo -e "${YELLOW}Opening summary in $EDITOR...${NC}"
    $EDITOR ${SUMMARY_FILE}
fi