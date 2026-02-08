# CAN Transport Capability

## ADDED Requirements

### Requirement: CAN-001

The C++ CLI SHALL support receiving ASTERIX data from Linux SocketCAN interfaces.

#### Scenario: Basic CAN reception
- **WHEN** the CLI is built with `-DENABLE_SOCKETCAN=ON`
- **AND** the user runs `./asterix -c vcan0 -j`
- **THEN** the CLI opens a SocketCAN connection to interface vcan0
- **AND** outputs parsed ASTERIX data as JSON

#### Scenario: CAN device not available
- **WHEN** the CLI is built with `-DENABLE_SOCKETCAN=ON`
- **AND** the user runs `./asterix -c nonexistent0 -j`
- **THEN** the CLI SHALL report an error that the interface cannot be opened

#### Scenario: CAN support not compiled
- **WHEN** the CLI is built WITHOUT `-DENABLE_SOCKETCAN=ON`
- **AND** the user runs `./asterix -c vcan0 -j`
- **THEN** the CLI SHALL report "SocketCAN support not compiled in"

### Requirement: CAN-002

The CAN device SHALL fragment large ASTERIX messages using the standard protocol matching the Rust implementation.

#### Scenario: Small message single frame
- **WHEN** an ASTERIX message of 7 bytes or less is transmitted over CAN
- **THEN** the message SHALL be sent in a single CAN frame with fragment header 0x80 (is_last)

#### Scenario: Large message multiple frames
- **WHEN** an ASTERIX message of 100 bytes is transmitted over Classic CAN (7-byte payload)
- **THEN** the message SHALL be split into ceil(100/7) = 15 frames
- **AND** each frame has a sequential fragment index (0-14)
- **AND** the last frame has the is_last flag (0x80) set

### Requirement: CAN-003

The CAN device SHALL reassemble fragmented ASTERIX messages from multiple CAN frames.

#### Scenario: Complete reassembly
- **WHEN** CAN frames arrive with fragment indices 0, 1, 2 for the same category
- **AND** the frame with is_last flag arrives
- **THEN** the fragments SHALL be concatenated in index order
- **AND** the complete ASTERIX message is returned

#### Scenario: Reassembly timeout
- **WHEN** fragments 0 and 1 arrive for a category
- **AND** no more fragments arrive within the reassembly timeout (default 1000ms)
- **THEN** the incomplete fragments SHALL be discarded
- **AND** memory is freed

#### Scenario: Out-of-order fragments
- **WHEN** fragments arrive in order: 2, 0, 1 (with is_last on fragment 2)
- **AND** all fragments have arrived
- **THEN** the fragments SHALL be correctly reassembled in order: 0, 1, 2

### Requirement: CAN-004

The CLI SHALL accept `-c` or `--can` option for CAN input.

#### Scenario: Basic interface specification
- **WHEN** the CLI supports CAN
- **AND** the user runs `./asterix -c can0`
- **THEN** the CLI SHALL use CAN interface "can0" with Classic CAN mode and 1000ms timeout

#### Scenario: CAN FD mode
- **WHEN** the CLI supports CAN
- **AND** the user runs `./asterix -c can0:fd`
- **THEN** the CLI SHALL use CAN FD mode (64-byte frames)

#### Scenario: Custom timeout
- **WHEN** the CLI supports CAN
- **AND** the user runs `./asterix -c can0:classic:2000`
- **THEN** the CLI SHALL use Classic CAN mode with 2000ms reassembly timeout

### Requirement: CAN-005

The build system SHALL support optional CAN compilation via CMake option.

#### Scenario: Enable CAN support
- **WHEN** CMake is run with `-DENABLE_SOCKETCAN=ON` on Linux with kernel CAN headers
- **THEN** `HAVE_SOCKETCAN` SHALL be defined
- **AND** `candevice.cxx` is compiled into the binary

#### Scenario: Disable CAN support default
- **WHEN** CMake is run without `-DENABLE_SOCKETCAN=ON`
- **THEN** CAN device code SHALL NOT be compiled
- **AND** the binary size is not increased

#### Scenario: Non-Linux platform
- **WHEN** CMake is run with `-DENABLE_SOCKETCAN=ON` on Windows or macOS
- **THEN** a warning SHALL be displayed that SocketCAN is Linux-only
- **AND** CAN support is disabled
