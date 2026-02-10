# Tasks: Add CAN Bus SocketCAN Support

## Status: Ready

_Validated 2026-02-07: All design references confirmed accurate. Gaps addressed (CAN ID limitation documented, Write() stub specified, CMake header check detailed, CI workflow target identified)._

## Prerequisites
- Linux system with kernel CAN support (NOT WSL2 standard kernel)
- For development: Native Linux VM or GitHub Actions CI

---

## Phase 1: Build System Setup

### Task 1.1: Add CMake ENABLE_SOCKETCAN option
**File**: `CMakeLists.txt`
**Validation**: `cmake -B build -DENABLE_SOCKETCAN=ON` shows "SocketCAN support: ON"

- [x] Add `option(ENABLE_SOCKETCAN "Enable SocketCAN transport support (Linux only)" OFF)`
- [x] Add `check_include_file("linux/can.h" HAVE_CAN_H)` via CMake `CheckIncludeFile` module
- [x] Add platform guard: warn and disable on non-Linux (Windows, macOS)
- [x] Add `HAVE_SOCKETCAN` compile definition when enabled and header found
- [x] Add `candevice.cxx` to source list conditionally
- [x] Add status output in build summary

---

## Phase 2: Device Implementation

### Task 2.1: Create CCanDevice header
**File**: `src/engine/candevice.hxx`
**Validation**: Compiles without errors

- [x] Include guards and feature guards (`#ifdef HAVE_SOCKETCAN`, `#ifndef _WIN32`)
- [x] Include SocketCAN headers (`linux/can.h`, `linux/can/raw.h`)
- [x] Define constants (CAN_PAYLOAD_SIZE=7, CANFD_PAYLOAD_SIZE=63)
- [x] Define CanFrameType enum (Classic, FD)
- [x] Define ReassemblyState struct with fragment map and timestamp
- [x] Declare CCanDevice class extending CBaseDevice
- [x] Declare static helper methods for CAN ID and fragment header encoding

### Task 2.2: Implement CCanDevice
**File**: `src/engine/candevice.cxx`
**Validation**: Unit tests pass

- [x] Constructor with CDescriptor parsing (`interface[:fd[:timeout]]`)
- [x] `Init()`: Create SocketCAN socket, bind to interface
- [x] `Read()`: Receive frames, reassemble fragments, return complete messages
- [x] `Write()`: No-op stub returning false (CAN output is out of scope, see proposal)
- [x] `Select()`: Timeout handling with select()
- [x] `CleanupExpiredStates()`: Remove stale reassembly states
- [x] Static helpers: `BuildCanId()`, `ParseCanId()`, `BuildFragmentHeader()`, `ParseFragmentHeader()`

### Task 2.3: Register CAN device in factory
**File**: `src/engine/devicefactory.cxx`
**Validation**: `./asterix --help` shows CAN option (when compiled)

- [x] Add `#include "candevice.hxx"` (guarded)
- [x] Add case for "can"/"socketcan" in `CreateDevice()`

---

## Phase 3: CLI Integration

### Task 3.1: Add CAN CLI option
**File**: `src/main/asterix.cpp`
**Validation**: `./asterix -c vcan0 -j` attempts to open vcan0

- [x] Add `strCANInput` variable
- [x] Add `-c`/`--can` argument parsing
- [x] Add help text explaining format: `interface[:fd[:timeout]]`
- [x] Build input string for CAN device in `buildInputString()`

---

## Phase 4: Testing

### Task 4.1: Create integration test script
**File**: `install/test/test_can.sh`
**Validation**: Script runs without errors on GitHub Actions

- [x] Check if SocketCAN compiled in (`--help | grep socketcan`)
- [x] Setup vcan0 interface (requires sudo)
- [x] Basic device initialization test
- [x] Cleanup vcan0 interface
- [x] Skip gracefully if vcan not available (WSL2)

### Task 4.2: Add CI workflow support
**File**: `.github/workflows/cross-platform-builds.yml` (Linux jobs only)
**Validation**: CI runs CAN tests on Linux Ubuntu runners

- [x] Add vcan kernel module setup step (`sudo modprobe vcan && sudo ip link add dev vcan0 type vcan && sudo ip link set up vcan0`)
- [x] Add `-DENABLE_SOCKETCAN=ON` to CMake configure for Linux jobs
- [x] Run test_can.sh in test phase
- [x] Skip CAN tests on Windows and macOS jobs

---

## Phase 5: Documentation

### Task 5.1: Update README with CAN usage
**Validation**: README shows CAN examples

- [x] Add CAN to supported transports list
- [x] Add CLI usage examples
- [x] Document vcan setup for testing
- [x] Note Linux-only limitation

### Task 5.2: Close GitHub issue #39
**Validation**: Issue #39 closed with implementation summary

- [ ] Add comment documenting completion
- [ ] Reference commit/PR
- [ ] Close issue

---

## Dependencies

```
Task 1.1 (CMake)
    └── Task 2.1 (Header)
         └── Task 2.2 (Implementation)
              └── Task 2.3 (Factory)
                   └── Task 3.1 (CLI)
                        └── Task 4.1 (Tests)
                             └── Task 4.2 (CI)
                                  └── Task 5.1 (Docs)
                                       └── Task 5.2 (Close issue)
```

## Parallel Work
- Tasks 5.1 (Docs) can be started after Task 3.1 (CLI) is complete
- CI setup (Task 4.2) can be prepared in parallel with implementation
