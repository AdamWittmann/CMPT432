#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PASSED=0
FAILED=0

echo "==================================="
echo "    LEXER TEST SUITE"
echo "==================================="
echo ""

# Test valid programs
echo "--- VALID PROGRAMS ---"
for file in tests/test-valid-*.txt; do
    echo -n "Testing $(basename $file)... "
    ./compiler "$file" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}PASS${NC}"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${NC}"
        ((FAILED++))
    fi
done

echo ""
echo "--- ERROR DETECTION ---"
for file in tests/test-error-*.txt; do
    echo -n "Testing $(basename $file)... "
    ./compiler "$file" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo -e "${GREEN}PASS${NC} (error detected)"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${NC} (should have errored)"
        ((FAILED++))
    fi
done

echo ""
echo "--- WARNING DETECTION ---"
for file in tests/test-warning-*.txt; do
    echo -n "Testing $(basename $file)... "
    OUTPUT=$(./compiler "$file" 2>&1)
    if echo "$OUTPUT" | grep -q "WARNING"; then
        echo -e "${GREEN}PASS${NC} (warning detected)"
        ((PASSED++))
    else
        echo -e "${YELLOW}FAIL${NC} (should have warned)"
        ((FAILED++))
    fi
done

echo ""
echo "==================================="
echo -e "Results: ${GREEN}${PASSED} passed${NC}, ${RED}${FAILED} failed${NC}"
echo "==================================="

exit $FAILED