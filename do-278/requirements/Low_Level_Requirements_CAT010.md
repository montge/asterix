# Low-Level Requirements - ASTERIX Category 010
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT010-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-010
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 010 (Transmission of Monosensor Surface Movement Data) v1.1.

**Parent Requirement:** REQ-HLR-CAT-010 - Parse ASTERIX Category 010

Each data item in CAT010 has corresponding LLRs defining parsing behavior.

---

## 2. CAT010 Data Item Requirements

### REQ-LLR-010-000: Parse Message Type (I010/000)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I010/000 (Message Type) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
This data item allows for a more convenient handling of the messages at the receiver side by further defining the type of transaction.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-000-001: Parse valid Message Type
- TC-010-000-002: Verify format compliance
- TC-010-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-010-010: Parse Data Source Identifier (I010/010)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I010/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Identification of the system from which the data are received.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-010-001: Parse valid Data Source Identifier
- TC-010-010-002: Verify format compliance
- TC-010-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-010-020: Parse Target Report Descriptor (I010/020)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I010/020 (Target Report Descriptor) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Type and characteristics of the data as transmitted by a system.

**Implementation Notes:**
- Format: Variable
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-020-001: Parse valid Target Report Descriptor
- TC-010-020-002: Verify format compliance
- TC-010-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-010-040: Parse Measured Position in Polar Co-ordinates. (I010/040)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/040 (Measured Position in Polar Co-ordinates.) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Measured Position of a target in local polar co-ordinates.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-040-001: Parse valid Measured Position in Polar Co-ordinates.
- TC-010-040-002: Verify format compliance
- TC-010-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-041: Parse Position in WGS-84 Coordinates (I010/041)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I010/041 (Position in WGS-84 Coordinates) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Position of a target in WGS-84 Coordinates.

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-041-001: Parse valid Position in WGS-84 Coordinates
- TC-010-041-002: Verify format compliance
- TC-010-041-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-010-042: Parse Position in Cartesian Coordinates (I010/042)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/042 (Position in Cartesian Coordinates) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Position of target in Cartesian Coordinates, in two’s complement form.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-042-001: Parse valid Position in Cartesian Coordinates
- TC-010-042-002: Verify format compliance
- TC-010-042-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-060: Parse Mode-3/A Code in Octal Representation (I010/060)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/060 (Mode-3/A Code in Octal Representation) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Mode-3/A code converted into octal representation.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-060-001: Parse valid Mode-3/A Code in Octal Representation
- TC-010-060-002: Verify format compliance
- TC-010-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-090: Parse Flight Level in Binary Representation (I010/090)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/090 (Flight Level in Binary Representation) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Flight Level (Mode S Altitude) converted into binary two's complement representation.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-090-001: Parse valid Flight Level in Binary Representation
- TC-010-090-002: Verify format compliance
- TC-010-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-091: Parse Measured Height (Local Cartesian Coordinates) (I010/091)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/091 (Measured Height (Local Cartesian Coordinates)) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Height above local 2D co-ordinate system in reference to the MLT System Reference Point as defined in item I019/610, in two’s complement form, based on a direct measurement not related to barometric pressure.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-091-001: Parse valid Measured Height (Local Cartesian Coordinates)
- TC-010-091-002: Verify format compliance
- TC-010-091-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-131: Parse Signal Amplitude (I010/131)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/131 (Signal Amplitude) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Relative strength of received signal.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-131-001: Parse valid Signal Amplitude
- TC-010-131-002: Verify format compliance
- TC-010-131-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-140: Parse Time of Day (I010/140)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I010/140 (Time of Day) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Absolute time stamping expressed as UTC.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-140-001: Parse valid Time of Day
- TC-010-140-002: Verify format compliance
- TC-010-140-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-010-161: Parse Track Number (I010/161)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/161 (Track Number) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
An integer value representing a unique reference to a track record within a particular track file.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-161-001: Parse valid Track Number
- TC-010-161-002: Verify format compliance
- TC-010-161-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-170: Parse Track Status (I010/170)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/170 (Track Status) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Status of track.

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-170-001: Parse valid Track Status
- TC-010-170-002: Verify format compliance
- TC-010-170-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-200: Parse Calculated Track Velocity in Polar Co-ordinates (I010/200)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/200 (Calculated Track Velocity in Polar Co-ordinates) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Calculated track velocity expressed in polar co-ordinates.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-200-001: Parse valid Calculated Track Velocity in Polar Co-ordinates
- TC-010-200-002: Verify format compliance
- TC-010-200-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-202: Parse Calculated Track Velocity in Cartesian Coordinates (I010/202)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/202 (Calculated Track Velocity in Cartesian Coordinates) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Calculated track velocity expressed in Cartesian Coordinates, in two’s complement representation.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-202-001: Parse valid Calculated Track Velocity in Cartesian Coordinates
- TC-010-202-002: Verify format compliance
- TC-010-202-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-210: Parse Calculated Acceleration (I010/210)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/210 (Calculated Acceleration) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Calculated Acceleration of the target, in two’s complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-210-001: Parse valid Calculated Acceleration
- TC-010-210-002: Verify format compliance
- TC-010-210-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-220: Parse Target Address (I010/220)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/220 (Target Address) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Target address (ICAO 24-bit address) assigned uniquely to each Target.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-220-001: Parse valid Target Address
- TC-010-220-002: Verify format compliance
- TC-010-220-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-245: Parse Target Identification (I010/245)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/245 (Target Identification) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Target (aircraft or vehicle) identification in 8 characters.

**Implementation Notes:**
- Format: Fixed (7 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-245-001: Parse valid Target Identification
- TC-010-245-002: Verify format compliance
- TC-010-245-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-250: Parse Mode S MB Data (I010/250)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/250 (Mode S MB Data) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Mode S Comm B data as extracted from the aircraft transponder.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-250-001: Parse valid Mode S MB Data
- TC-010-250-002: Verify format compliance
- TC-010-250-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-270: Parse Target Size and Orientation (I010/270)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/270 (Target Size and Orientation) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Target size defined as length and with of the detected target, and orientation.

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-270-001: Parse valid Target Size and Orientation
- TC-010-270-002: Verify format compliance
- TC-010-270-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-280: Parse Special Purpose (I010/280)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/280 (Special Purpose) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Special information

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-280-001: Parse valid Special Purpose
- TC-010-280-002: Verify format compliance
- TC-010-280-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-300: Parse Vehicle Fleet Identification (I010/300)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/300 (Vehicle Fleet Identification) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Vehicle fleet identification number.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-300-001: Parse valid Vehicle Fleet Identification
- TC-010-300-002: Verify format compliance
- TC-010-300-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-310: Parse Pre-programmed Message (I010/310)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/310 (Pre-programmed Message) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Number related to a pre-programmed message that can be transmitted by a vehicle.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-310-001: Parse valid Pre-programmed Message
- TC-010-310-002: Verify format compliance
- TC-010-310-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-500: Parse Standard Deviation of POsition (I010/500)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/500 (Standard Deviation of POsition) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Standard Deviation of Position

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-500-001: Parse valid Standard Deviation of POsition
- TC-010-500-002: Verify format compliance
- TC-010-500-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-550: Parse System Status (I010/550)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/550 (System Status) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Iformation concerning the configuration and status of a System.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-550-001: Parse valid System Status
- TC-010-550-002: Verify format compliance
- TC-010-550-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-RE: Parse Reserved Expansion Field (I010/RE)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/RE (Reserved Expansion Field) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**

            Expansion
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-RE-001: Parse valid Reserved Expansion Field
- TC-010-RE-002: Verify format compliance
- TC-010-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-010-SP: Parse Special Purpose (I010/SP)

**Parent:** REQ-HLR-CAT-010
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I010/SP (Special Purpose) as specified in the EUROCONTROL ASTERIX CAT010 v1.1 specification.

**Data Item Definition:**
Special information

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-SP-001: Parse valid Special Purpose
- TC-010-SP-002: Verify format compliance
- TC-010-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat010_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-010-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT010 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-ERR-001: Truncated data items
- TC-010-ERR-002: Invalid repetition counts
- TC-010-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-010-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT010 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-010-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-010-000 | I010/000 | Message Type | Critical | High |
| REQ-LLR-010-010 | I010/010 | Data Source Identifier | Critical | High |
| REQ-LLR-010-020 | I010/020 | Target Report Descriptor | Critical | High |
| REQ-LLR-010-040 | I010/040 | Measured Position in Polar Co-ordinates. | Medium | Low |
| REQ-LLR-010-041 | I010/041 | Position in WGS-84 Coordinates | Critical | High |
| REQ-LLR-010-042 | I010/042 | Position in Cartesian Coordinates | Medium | Low |
| REQ-LLR-010-060 | I010/060 | Mode-3/A Code in Octal Representation | Medium | Low |
| REQ-LLR-010-090 | I010/090 | Flight Level in Binary Representation | Medium | Low |
| REQ-LLR-010-091 | I010/091 | Measured Height (Local Cartesian Coordin | Medium | Low |
| REQ-LLR-010-131 | I010/131 | Signal Amplitude | Medium | Low |
| REQ-LLR-010-140 | I010/140 | Time of Day | Critical | High |
| REQ-LLR-010-161 | I010/161 | Track Number | Medium | Low |
| REQ-LLR-010-170 | I010/170 | Track Status | Medium | Low |
| REQ-LLR-010-200 | I010/200 | Calculated Track Velocity in Polar Co-or | Medium | Low |
| REQ-LLR-010-202 | I010/202 | Calculated Track Velocity in Cartesian C | Medium | Low |
| REQ-LLR-010-210 | I010/210 | Calculated Acceleration | Medium | Low |
| REQ-LLR-010-220 | I010/220 | Target Address | Medium | Low |
| REQ-LLR-010-245 | I010/245 | Target Identification | Medium | Low |
| REQ-LLR-010-250 | I010/250 | Mode S MB Data | Medium | Low |
| REQ-LLR-010-270 | I010/270 | Target Size and Orientation | Medium | Low |
| REQ-LLR-010-280 | I010/280 | Special Purpose | Medium | Low |
| REQ-LLR-010-300 | I010/300 | Vehicle Fleet Identification | Medium | Low |
| REQ-LLR-010-310 | I010/310 | Pre-programmed Message | Medium | Low |
| REQ-LLR-010-500 | I010/500 | Standard Deviation of POsition | Medium | Low |
| REQ-LLR-010-550 | I010/550 | System Status | Medium | Low |
| REQ-LLR-010-RE | I010/RE | Reserved Expansion Field | Medium | Low |
| REQ-LLR-010-SP | I010/SP | Special Purpose | Medium | Low |
| REQ-LLR-010-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-010-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 29

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-010
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-010-xxx-xxx)

See Requirements Traceability Matrix (RTM) for complete mapping.

---

## Document Approval

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Requirements Engineer | TBD | 2025-11-02 | |
| Developer | TBD | | |
| QA Manager | TBD | | |

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT010 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
