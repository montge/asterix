#!/bin/bash
#
# Ada code quality checks for ASTERIX Ada bindings
# Run this before committing changes
#
# Usage: ./scripts/check.sh [--fix]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check for GNATprove in common locations
GNATPROVE=""
if command -v gnatprove &> /dev/null; then
    GNATPROVE="gnatprove"
elif [ -x "$HOME/.local/share/alire/releases/gnatprove_13.2.1_28fc3583/bin/gnatprove" ]; then
    GNATPROVE="$HOME/.local/share/alire/releases/gnatprove_13.2.1_28fc3583/bin/gnatprove"
fi

echo "========================================"
echo "  ASTERIX Ada Code Quality Checks"
echo "========================================"
echo ""

ERRORS=0

# 1. Syntax and type checking (compile with all warnings)
echo -e "${YELLOW}[1/5] Compiling with strict warnings...${NC}"
if gprbuild -P asterix_decoder.gpr -XASTERIX_BUILD_MODE=debug -c -q 2>&1; then
    echo -e "${GREEN}  ✓ Compilation passed${NC}"
else
    echo -e "${RED}  ✗ Compilation failed${NC}"
    ERRORS=$((ERRORS + 1))
fi
echo ""

# 2. Build the library
echo -e "${YELLOW}[2/5] Building library...${NC}"
if gprbuild -P asterix_decoder.gpr -XASTERIX_BUILD_MODE=debug -q 2>&1; then
    echo -e "${GREEN}  ✓ Library build passed${NC}"
else
    echo -e "${RED}  ✗ Library build failed${NC}"
    ERRORS=$((ERRORS + 1))
fi
echo ""

# 3. SPARK verification (if GNATprove available)
echo -e "${YELLOW}[3/5] Running SPARK verification...${NC}"
if [ -n "$GNATPROVE" ]; then
    if $GNATPROVE -P asterix_decoder.gpr --mode=flow -q 2>&1; then
        echo -e "${GREEN}  ✓ SPARK flow analysis passed${NC}"
    else
        echo -e "${RED}  ✗ SPARK flow analysis failed${NC}"
        ERRORS=$((ERRORS + 1))
    fi
else
    echo -e "${YELLOW}  ⚠ GNATprove not found (install via: alr with gnatprove)${NC}"
fi
echo ""

# 4. Check for SPARK annotations (informational)
echo -e "${YELLOW}[4/5] Checking for SPARK annotations...${NC}"
SPARK_COUNT=$(grep -r "SPARK_Mode" src/ 2>/dev/null | wc -l || echo 0)
if [ "$SPARK_COUNT" -gt 0 ]; then
    echo -e "${GREEN}  ✓ Found $SPARK_COUNT SPARK annotations${NC}"
else
    echo -e "${YELLOW}  ⚠ No SPARK annotations found (optional)${NC}"
fi
echo ""

# 5. Check line lengths (79 char max per GNAT style)
echo -e "${YELLOW}[5/5] Checking line lengths (max 79 chars)...${NC}"
LONG_LINES=$(find src/ -name "*.ad[sb]" -exec awk 'length > 79 {print FILENAME ":" NR ": line too long (" length " chars)"}' {} \; 2>/dev/null | head -10)
if [ -z "$LONG_LINES" ]; then
    echo -e "${GREEN}  ✓ All lines within 79 characters${NC}"
else
    echo -e "${RED}  ✗ Lines exceeding 79 characters:${NC}"
    echo "$LONG_LINES"
    ERRORS=$((ERRORS + 1))
fi
echo ""

# Summary
echo "========================================"
if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}All checks passed!${NC}"
    exit 0
else
    echo -e "${RED}$ERRORS check(s) failed${NC}"
    exit 1
fi
