#!/bin/bash
#
# CAN Bus / SocketCAN Integration Tests
#
# Tests the SocketCAN transport layer for receiving ASTERIX data over CAN bus.
# Requires Linux with vcan kernel module support.
#
# Usage: ./test_can.sh
#

failedtests=0
passedtests=0
skippedtests=0

exec="../bin/asterix"
config="../share/asterix/config/asterix.ini"

# Verify executable exists
if [ ! -f "${exec}" ]; then
    echo "Error: Executable not found at ${exec}"
    echo "Current directory: $(pwd)"
    exit 1
fi

pass_test() {
    echo "OK       $1"
    passedtests=$((passedtests + 1))
}

fail_test() {
    echo "FAILED   $1"
    failedtests=$((failedtests + 1))
}

skip_test() {
    echo "SKIPPED  $1"
    skippedtests=$((skippedtests + 1))
}

echo "=========================================="
echo "  CAN Bus / SocketCAN Integration Tests"
echo "=========================================="
echo ""

# Test 1: Check if SocketCAN support is compiled in
echo "--- Test 1: SocketCAN compile-time support ---"
if ${exec} --help 2>&1 | grep -qi "can"; then
    pass_test "SocketCAN support compiled in"
    CAN_COMPILED=1
else
    skip_test "SocketCAN support not compiled in (rebuild with -DENABLE_SOCKETCAN=ON)"
    CAN_COMPILED=0
fi

# If CAN is not compiled in, skip remaining tests
if [ "${CAN_COMPILED}" -eq 0 ]; then
    echo ""
    echo "=========================================="
    echo "  CAN tests skipped (not compiled in)"
    echo "  Passed: ${passedtests}  Failed: ${failedtests}  Skipped: ${skippedtests}"
    echo "=========================================="
    exit 0
fi

# Test 2: Check CAN argument is accepted (should fail gracefully without interface)
echo "--- Test 2: CAN argument parsing ---"
output=$(${exec} -d ${config} -c nonexistent_iface -j 2>&1)
exitcode=$?
# We expect a failure (non-zero exit) because the interface does not exist,
# but it should NOT fail with "unknown option" or "not compiled in"
if echo "${output}" | grep -qi "not compiled in"; then
    fail_test "CAN argument rejected as not compiled in"
elif echo "${output}" | grep -qi "unknown"; then
    fail_test "CAN argument not recognized"
else
    pass_test "CAN argument accepted by CLI parser"
fi

# Test 3: Check vcan kernel module availability
echo "--- Test 3: vcan kernel module availability ---"
VCAN_AVAILABLE=0
if [ "$(uname -s)" != "Linux" ]; then
    skip_test "Not on Linux - vcan not available"
elif ! modinfo vcan > /dev/null 2>&1; then
    skip_test "vcan kernel module not available (WSL2 or minimal kernel)"
else
    pass_test "vcan kernel module available"
    VCAN_AVAILABLE=1
fi

# Test 4: Setup vcan0 interface and test device initialization
echo "--- Test 4: vcan0 interface setup and device init ---"
VCAN_SETUP=0
if [ "${VCAN_AVAILABLE}" -eq 0 ]; then
    skip_test "vcan not available - skipping device init test"
else
    # Try to load vcan module and create interface
    sudo modprobe vcan 2>/dev/null
    sudo ip link add dev vcan0 type vcan 2>/dev/null
    sudo ip link set up vcan0 2>/dev/null

    if ip link show vcan0 > /dev/null 2>&1; then
        VCAN_SETUP=1
        pass_test "vcan0 interface created and up"
    else
        skip_test "Failed to create vcan0 (insufficient privileges)"
    fi
fi

# Test 5: Attempt CAN device connection on vcan0
echo "--- Test 5: CAN device connection test ---"
if [ "${VCAN_SETUP}" -eq 0 ]; then
    skip_test "vcan0 not available - skipping connection test"
else
    # Run asterix with CAN input on vcan0, with a short timeout
    # It should open the socket successfully but timeout waiting for data
    timeout 2 ${exec} -d ${config} -c vcan0 -j 2>&1
    exitcode=$?
    # timeout returns 124 on timeout, which means the process started successfully
    # and was waiting for data (expected behavior)
    if [ ${exitcode} -eq 124 ]; then
        pass_test "CAN device opened vcan0 successfully (timed out waiting for data)"
    elif [ ${exitcode} -eq 0 ]; then
        pass_test "CAN device connected to vcan0"
    else
        fail_test "CAN device failed to open vcan0 (exit code: ${exitcode})"
    fi
fi

# Test 6: CAN FD mode argument parsing
echo "--- Test 6: CAN FD argument format ---"
if [ "${VCAN_SETUP}" -eq 0 ]; then
    skip_test "vcan0 not available - skipping FD test"
else
    timeout 2 ${exec} -d ${config} -c vcan0:fd -j 2>&1
    exitcode=$?
    if [ ${exitcode} -eq 124 ] || [ ${exitcode} -eq 0 ]; then
        pass_test "CAN FD mode argument accepted"
    else
        fail_test "CAN FD mode argument rejected (exit code: ${exitcode})"
    fi
fi

# Test 7: CAN with timeout parameter
echo "--- Test 7: CAN timeout parameter ---"
if [ "${VCAN_SETUP}" -eq 0 ]; then
    skip_test "vcan0 not available - skipping timeout test"
else
    timeout 2 ${exec} -d ${config} -c vcan0:classic:2000 -j 2>&1
    exitcode=$?
    if [ ${exitcode} -eq 124 ] || [ ${exitcode} -eq 0 ]; then
        pass_test "CAN timeout parameter accepted"
    else
        fail_test "CAN timeout parameter rejected (exit code: ${exitcode})"
    fi
fi

# Cleanup: Remove vcan0 interface
if [ "${VCAN_SETUP}" -eq 1 ]; then
    echo ""
    echo "--- Cleanup ---"
    sudo ip link set down vcan0 2>/dev/null
    sudo ip link delete vcan0 2>/dev/null
    echo "vcan0 interface removed"
fi

# Summary
echo ""
echo "=========================================="
echo "  CAN Bus Test Summary"
echo "  Passed: ${passedtests}  Failed: ${failedtests}  Skipped: ${skippedtests}"
echo "=========================================="

if [ ${failedtests} -gt 0 ]; then
    exit 1
fi
exit 0
