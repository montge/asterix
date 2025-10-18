# Low-Level Requirements - ASTERIX Category 048
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT048-001
**Revision:** 1.0
**Date:** 2025-10-17
**Parent HLR:** REQ-HLR-CAT-048
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 048 (Transmission of Monoradar Target Reports) v1.21.

**Parent Requirement:** REQ-HLR-CAT-048 - Parse ASTERIX Category 048

Each data item in CAT048 has corresponding LLRs defining parsing behavior.

---

## 2. CAT048 Data Item Requirements

### REQ-LLR-048-010: Parse Data Source Identifier (I048/010)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I048/010 (Data Source Identifier) as a 2-byte fixed-length field containing:
- SAC (System Area Code): Byte 1, bits 16-9, unsigned integer (0-255)
- SIC (System Identification Code): Byte 2, bits 8-1, unsigned integer (0-255)

**Implementation Notes:**
- Format: Fixed, 2 bytes
- No scaling required (raw values)
- Identifies the radar station from which data are received

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-010-001: Parse valid SAC/SIC values
- TC-048-010-002: Parse boundary values (0x00, 0xFF)
- TC-048-010-003: Verify correct byte order

**Code Reference:**
- src/asterix/DataItemFormatFixed.cpp
- asterix/config/asterix_cat048_1_21.xml (lines 24-39)

**Design Reference:** SDD Section 3.4.2 (Fixed Format Parsing)

**Safety Impact:** High (incorrect SAC/SIC misidentifies data source)

---

### REQ-LLR-048-020: Parse Target Report Descriptor (I048/020)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I048/020 (Target Report Descriptor) as a variable-length field (1-2 bytes) containing target type and characteristics:

**First Byte (always present):**
- TYP (bits 8-6): Target type (3 bits)
  - 0: No detection
  - 1: Single PSR detection
  - 2: Single SSR detection
  - 3: PSR+SSR detection
  - 4: Single Mode S All-Call
  - 5: Single Mode S Roll-Call
  - 6: PSR+Mode S All-Call
  - 7: PSR+Mode S Roll-Call
- SIM (bit 5): Simulated/Actual (0=Actual, 1=Simulated)
- RDP (bit 4): RDP Chain (0=Chain 1, 1=Chain 2)
- SPI (bit 3): Special Position Identification (0=Absence, 1=SPI)
- RAB (bit 2): Report from Aircraft Transponder (0=Aircraft, 1=Fixed transponder)
- FX (bit 1): Extension flag (0=End, 1=Extension)

**Second Byte (if FX=1):**
- TST (bit 8): Test target (0=Real, 1=Test)
- Spare bits (set to 0)
- FX (bit 1): Further extension

**Implementation Notes:**
- Format: Variable length (1 or 2 bytes depending on FX bit)
- Parse first byte, check FX bit
- If FX=1, parse second byte
- Continue until FX=0

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-020-001: Parse 1-byte descriptor (FX=0)
- TC-048-020-002: Parse 2-byte descriptor (FX=1)
- TC-048-020-003: Verify all TYP codes (0-7)
- TC-048-020-004: Verify SIM, RDP, SPI, RAB bits
- TC-048-020-005: Verify TST bit in extension

**Code Reference:**
- src/asterix/DataItemFormatVariable.cpp
- asterix/config/asterix_cat048_1_21.xml (lines 41-141)

**Design Reference:** SDD Section 3.4.3 (Variable Format Parsing)

**Safety Impact:** High (target type affects tracking)

---

### REQ-LLR-048-040: Parse Measured Position in Polar Coordinates (I048/040)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I048/040 (Measured Position in Polar Coordinates) as a 4-byte fixed-length field containing:
- RHO (bytes 1-2): Range in polar coordinates (unsigned 16-bit)
  - Resolution: 1/256 NM (approximately 7.8 meters)
  - Range: 0 to 256 NM
- THETA (bytes 3-4): Azimuth in polar coordinates (unsigned 16-bit)
  - Resolution: 360°/2^16 (approximately 0.0055°)
  - Range: 0° to 360°

**Scaling:**
- RHO (NM) = Raw Value × (1/256)
- THETA (degrees) = Raw Value × (360/65536)

**Implementation Notes:**
- Format: Fixed, 4 bytes
- Big-endian byte order
- Position measured by the radar

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-040-001: Parse known position values
- TC-048-040-002: Verify scaling calculations
- TC-048-040-003: Verify boundary values (0, max)
- TC-048-040-004: Compare with reference implementation

**Code Reference:**
- src/asterix/DataItemFormatFixed.cpp
- src/asterix/DataItemBits.cpp (scaling logic)
- asterix/config/asterix_cat048_1_21.xml

**Design Reference:** SDD Section 3.4.2 (Fixed Format with Scaling)

**Safety Impact:** High (position data critical for track accuracy)

---

### REQ-LLR-048-070: Parse Mode-3/A Code (I048/070)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I048/070 (Mode-3/A Code in Octal Representation) as a 2-byte fixed-length field containing:
- V (bit 16): Validated (0=Code validated, 1=Code not validated)
- G (bit 15): Garbled (0=Default, 1=Garbled code)
- L (bit 14): Local track (0=Default, 1=Local track)
- Spare (bit 13): Set to 0
- Mode-3/A Code (bits 12-1): 12-bit octal code (4 octal digits)

**Mode-3/A Code Representation:**
- 4 octal digits: D4 D3 D2 D1
- Each digit: 0-7 (3 bits)
- Bits 12-10: D4, Bits 9-7: D3, Bits 6-4: D2, Bits 3-1: D1

**Implementation Notes:**
- Format: Fixed, 2 bytes
- Extract octal representation
- Check V, G, L flags

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-070-001: Parse valid Mode-3/A codes
- TC-048-070-002: Verify V, G, L flags
- TC-048-070-003: Validate octal representation
- TC-048-070-004: Test special codes (7700, 7600, 7500)

**Code Reference:**
- src/asterix/DataItemFormatFixed.cpp
- asterix/config/asterix_cat048_1_21.xml

**Design Reference:** SDD Section 3.4.2

**Safety Impact:** High (Mode-3/A code identifies aircraft)

---

### REQ-LLR-048-140: Parse Time Of Day (I048/140)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I048/140 (Time Of Day) as a 3-byte fixed-length field containing:
- Time of day in seconds since midnight (unsigned 24-bit)
- Resolution: 1/128 seconds
- Range: 0 to 86400 seconds (24 hours)

**Scaling:**
- Time (seconds) = Raw Value × (1/128)

**Implementation Notes:**
- Format: Fixed, 3 bytes
- Absolute time stamp (UTC)
- Used for correlation and replay

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-140-001: Parse known time values
- TC-048-140-002: Verify scaling to seconds
- TC-048-140-003: Verify midnight (0) and day boundary
- TC-048-140-004: Verify fractional seconds accuracy

**Code Reference:**
- src/asterix/DataItemFormatFixed.cpp
- asterix/config/asterix_cat048_1_21.xml

**Design Reference:** SDD Section 3.4.2

**Safety Impact:** Medium (timing affects correlation)

---

### REQ-LLR-048-240: Parse Aircraft Identification (I048/240)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/240 (Aircraft Identification) as a 6-byte fixed-length field containing:
- Aircraft identification (callsign) in 6-bit coded characters
- 8 characters × 6 bits = 48 bits = 6 bytes
- Character set: A-Z, 0-9, space

**6-bit Character Encoding:**
- 1-26: A-Z
- 48-57: 0-9
- 32: Space

**Implementation Notes:**
- Format: Fixed, 6 bytes
- Decode each 6-bit value to ASCII character
- Trim trailing spaces

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-240-001: Parse valid callsigns
- TC-048-240-002: Verify character decoding
- TC-048-240-003: Handle spaces and special characters
- TC-048-240-004: Verify 8-character limit

**Code Reference:**
- src/asterix/DataItemFormatFixed.cpp
- src/asterix/DataItemBits.cpp (character decoding)
- asterix/config/asterix_cat048_1_21.xml

**Design Reference:** SDD Section 3.4.2

**Safety Impact:** Medium (callsign for identification)

---

### REQ-LLR-048-090: Parse Flight Level (I048/090)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/090 (Flight Level in Binary Representation) as a 2-byte fixed-length field containing:
- V (bit 16): Validated (0=Code validated, 1=Code not validated)
- G (bit 15): Garbled (0=Default, 1=Garbled)
- Flight Level (bits 14-1): Signed 14-bit value in binary two's complement
- Resolution: 1/4 FL (25 feet)
- Range: -12 FL to +1270 FL

**Scaling:**
- Flight Level (FL) = Raw Value × 0.25

**Implementation Notes:**
- Format: Fixed, 2 bytes
- Signed value (two's complement)
- V and G flags indicate validity

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-090-001: Parse positive flight levels
- TC-048-090-002: Parse negative flight levels (if supported)
- TC-048-090-003: Verify V, G flags
- TC-048-090-004: Verify scaling to FL

**Code Reference:**
- src/asterix/DataItemFormatFixed.cpp
- asterix/config/asterix_cat048_1_21.xml

**Design Reference:** SDD Section 3.4.2

**Safety Impact:** High (altitude critical for separation)

---

### REQ-LLR-048-220: Parse Aircraft Address (I048/220)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/220 (Aircraft Address) as a 3-byte fixed-length field containing:
- 24-bit Mode S address (ICAO 24-bit address)
- Unsigned integer
- Range: 0x000000 to 0xFFFFFF

**Implementation Notes:**
- Format: Fixed, 3 bytes
- Unique aircraft identifier in Mode S
- Used for correlation and tracking

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-220-001: Parse valid Mode S addresses
- TC-048-220-002: Verify 24-bit range
- TC-048-220-003: Test known addresses

**Code Reference:**
- src/asterix/DataItemFormatFixed.cpp
- asterix/config/asterix_cat048_1_21.xml

**Design Reference:** SDD Section 3.4.2

**Safety Impact:** High (unique identifier for aircraft)

---

### REQ-LLR-048-161: Parse Track Number (I048/161)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/161 (Track Number) as a 2-byte fixed-length field containing:
- Spare (bits 16-13): Set to 0
- Track Number (bits 12-1): 12-bit unsigned integer
- Range: 0 to 4095

**Implementation Notes:**
- Format: Fixed, 2 bytes
- Identifies the track within the system
- Unique within data source

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-161-001: Parse valid track numbers
- TC-048-161-002: Verify 12-bit range (0-4095)
- TC-048-161-003: Verify spare bits ignored

**Code Reference:**
- src/asterix/DataItemFormatFixed.cpp
- asterix/config/asterix_cat048_1_21.xml

**Design Reference:** SDD Section 3.4.2

**Safety Impact:** Medium (track identification)

---

## 3. Complex Data Items

### REQ-LLR-048-120: Parse Radial Doppler Speed (I048/120)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I048/120 (Radial Doppler Speed) as a repetitive data item where:
- First byte: REP (repetition factor), indicates number of speed values (1-255)
- Followed by REP × 2-byte speed values

**Each speed value (2 bytes):**
- CAL (bit 16): Calculated (0=Doppler speed, 1=Calculated speed)
- Spare (bits 15-11): Set to 0
- Speed (bits 10-1): Signed 10-bit value in two's complement
- Resolution: 1 m/s (approx 2 knots)
- Range: -512 to +511 m/s

**Implementation Notes:**
- Format: Repetitive with 2-byte fixed elements
- Process REP iterations
- Handle signed values

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-120-001: Parse single speed value (REP=1)
- TC-048-120-002: Parse multiple speed values (REP>1)
- TC-048-120-003: Verify signed value handling
- TC-048-120-004: Verify CAL flag

**Code Reference:**
- src/asterix/DataItemFormatRepetitive.cpp
- asterix/config/asterix_cat048_1_21.xml

**Design Reference:** SDD Section 3.4.4 (Repetitive Format)

**Safety Impact:** Medium (velocity data for tracking)

---

### REQ-LLR-048-250: Parse Mode S Comm B Data (I048/250)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I048/250 (BDS data) as a repetitive data item where:
- First byte: REP (repetition factor), indicates number of MB data segments
- Each segment: 8 bytes (64 bits) of Mode S Comm-B data (BDS)

**BDS Data:**
- Contains various aircraft-derived data
- Format depends on BDS register code
- May include: airspeed, heading, vertical rate, roll angle, track angle, ground speed, etc.

**Implementation Notes:**
- Format: Repetitive with 8-byte fixed elements
- BDS decoding defined in separate BDS configuration
- Requires BDS register identification

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-250-001: Parse single BDS segment
- TC-048-250-002: Parse multiple BDS segments
- TC-048-250-003: Verify 8-byte segment length

**Code Reference:**
- src/asterix/DataItemFormatRepetitive.cpp
- src/asterix/DataItemFormatBDS.cpp (BDS decoding)
- asterix/config/asterix_bds.xml (BDS definitions)

**Design Reference:** SDD Section 3.4.4, 3.4.6 (BDS Format)

**Safety Impact:** Medium (downlinked aircraft data)

---

### REQ-LLR-048-260: Parse ACAS Resolution Advisory (I048/260)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/260 (ACAS Resolution Advisory Report) as a 7-byte fixed-length field containing ACAS (Airborne Collision Avoidance System) resolution advisory information.

**Implementation Notes:**
- Format: Fixed, 7 bytes (56 bits)
- Contains RA active flags, RA type, threat identity, etc.
- Critical for collision avoidance

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-260-001: Parse ACAS RA data
- TC-048-260-002: Verify field extraction

**Code Reference:**
- src/asterix/DataItemFormatFixed.cpp
- asterix/config/asterix_cat048_1_21.xml

**Design Reference:** SDD Section 3.4.2

**Safety Impact:** High (ACAS data critical for collision avoidance)

---

## 4. Special Data Items

### REQ-LLR-048-SP: Parse Special Purpose Field (I048/SP)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** Low

**Description:**
The parser shall extract Data Item I048/SP (Special Purpose Field) as an explicit-length data item where:
- First byte: LEN (length in bytes)
- Followed by LEN bytes of user-defined data

**Implementation Notes:**
- Format: Explicit (variable length with length indicator)
- Content not standardized
- Implementation-specific

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-SP-001: Parse SP field with various lengths

**Code Reference:**
- src/asterix/DataItemFormatExplicit.cpp

**Design Reference:** SDD Section 3.4.5 (Explicit Format)

**Safety Impact:** Low (non-standard data)

---

## 5. Error Handling Requirements

### REQ-LLR-048-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT048 data item has invalid length:
- Fixed items: Length not matching specification
- Variable items: FX bit sequence errors
- Repetitive items: REP value causes overflow
- Explicit items: LEN value exceeds available data

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-ERR-001: Truncated data item
- TC-048-ERR-002: Invalid repetition count

**Code Reference:**
- src/asterix/DataRecord.cpp (error detection)

**Safety Impact:** High (prevents incorrect parsing)

---

### REQ-LLR-048-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT048 data items not defined in the configuration, skipping the item and continuing with the next.

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-ERR-003: Unknown data item in FSPEC

**Safety Impact:** Medium (robustness)

---

## 6. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-048-010 | I048/010 | Data Source Identifier | Critical | High |
| REQ-LLR-048-020 | I048/020 | Target Report Descriptor | Critical | High |
| REQ-LLR-048-040 | I048/040 | Measured Position (Polar) | Critical | High |
| REQ-LLR-048-070 | I048/070 | Mode-3/A Code | Critical | High |
| REQ-LLR-048-090 | I048/090 | Flight Level | High | High |
| REQ-LLR-048-140 | I048/140 | Time Of Day | Critical | Medium |
| REQ-LLR-048-161 | I048/161 | Track Number | High | Medium |
| REQ-LLR-048-220 | I048/220 | Aircraft Address | High | High |
| REQ-LLR-048-240 | I048/240 | Aircraft Identification | High | Medium |
| REQ-LLR-048-120 | I048/120 | Radial Doppler Speed | Medium | Medium |
| REQ-LLR-048-250 | I048/250 | Mode S Comm B Data | Medium | Medium |
| REQ-LLR-048-260 | I048/260 | ACAS Resolution Advisory | High | High |
| REQ-LLR-048-SP | I048/SP | Special Purpose Field | Low | Low |
| REQ-LLR-048-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-048-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Note:** Full LLR documentation should include all 27 CAT048 data items. This document shows representative examples. Remaining data items (030, 042, 050, 055, 060, 065, 080, 100, 110, 130, 170, 200, 210, 230) follow similar structure.

---

## 7. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-048
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-048-xxx-xxx)

See Requirements Traceability Matrix (RTM) for complete mapping.

---

## Document Approval

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Requirements Engineer | TBD | 2025-10-17 | |
| Developer | TBD | | |
| QA Manager | TBD | | |

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-10-17 | Initial | Initial LLR for CAT048 key data items |

---

## Next Actions

1. Complete LLR for remaining 12 CAT048 data items
2. Create test cases for each LLR
3. Link LLR to existing code
4. Update Requirements Traceability Matrix
5. Begin implementation of missing tests
