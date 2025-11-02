# Low-Level Requirements - ASTERIX Category 020
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT020-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-020
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 020 (Multilateration Target Reports) v1.10.

**Parent Requirement:** REQ-HLR-CAT-020 - Parse ASTERIX Category 020

Each data item in CAT020 has corresponding LLRs defining parsing behavior.

---

## 2. CAT020 Data Item Requirements

### REQ-LLR-020-010: Parse Data Source Identifier (I020/010)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I020/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Identification of the system from which the data are received.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-010-001: Parse valid Data Source Identifier
- TC-020-010-002: Verify format compliance
- TC-020-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-020-020: Parse Target Report Descriptor (I020/020)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I020/020 (Target Report Descriptor) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Type and characteristics of the data as transmitted by a system.

**Implementation Notes:**
- Format: Variable
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-020-001: Parse valid Target Report Descriptor
- TC-020-020-002: Verify format compliance
- TC-020-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-020-030: Parse Warning/Error Conditions (I020/030)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/030 (Warning/Error Conditions) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Warning/error conditions detected by a system for the target report involved.

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-030-001: Parse valid Warning/Error Conditions
- TC-020-030-002: Verify format compliance
- TC-020-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-041: Parse Position in WGS-84 Coordinates (I020/041)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I020/041 (Position in WGS-84 Coordinates) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Position of a target in WGS-84 Coordinates.

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-041-001: Parse valid Position in WGS-84 Coordinates
- TC-020-041-002: Verify format compliance
- TC-020-041-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-020-042: Parse Position in Cartesian Coordinates (I020/042)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/042 (Position in Cartesian Coordinates) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Calculated position in Cartesian Coordinates, in two's complement representation.

**Implementation Notes:**
- Format: Fixed (6 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-042-001: Parse valid Position in Cartesian Coordinates
- TC-020-042-002: Verify format compliance
- TC-020-042-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-050: Parse Mode-2 Code in Octal Representation (I020/050)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/050 (Mode-2 Code in Octal Representation) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Mode-2 code converted into octal representation.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-050-001: Parse valid Mode-2 Code in Octal Representation
- TC-020-050-002: Verify format compliance
- TC-020-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-055: Parse Mode-1 Code in Octal Representation (I020/055)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/055 (Mode-1 Code in Octal Representation) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Mode-1 code converted into octal representation.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-055-001: Parse valid Mode-1 Code in Octal Representation
- TC-020-055-002: Verify format compliance
- TC-020-055-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-070: Parse Mode-3/A Code in Octal Representation (I020/070)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/070 (Mode-3/A Code in Octal Representation) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Mode-3/A code converted into octal representation.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-070-001: Parse valid Mode-3/A Code in Octal Representation
- TC-020-070-002: Verify format compliance
- TC-020-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-090: Parse Flight Level in Binary Representation (I020/090)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/090 (Flight Level in Binary Representation) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Flight Level (Mode S Altitude) converted into binary two's complement representation.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-090-001: Parse valid Flight Level in Binary Representation
- TC-020-090-002: Verify format compliance
- TC-020-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-100: Parse Mode-C Code (I020/100)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/100 (Mode-C Code) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Mode-C height in Gray notation as received from the transponder together with the confidence level for each reply bit as provided by a MSSR/Mode-S station.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-100-001: Parse valid Mode-C Code
- TC-020-100-002: Verify format compliance
- TC-020-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-105: Parse Geometric Height (WGS-84) (I020/105)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/105 (Geometric Height (WGS-84)) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Vertical distance between the target and the projection of its position on the earth's ellipsoid, as defined by WGS84, in two's complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-105-001: Parse valid Geometric Height (WGS-84)
- TC-020-105-002: Verify format compliance
- TC-020-105-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-110: Parse Measured Height (Local Cartesian Coordinates) (I020/110)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/110 (Measured Height (Local Cartesian Coordinates)) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Height above local 2D co-ordinate system in reference to the MLT System Reference Point as defined in item I019/610, in two's complement form, based on a direct measurement not related to barometric pressure.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-110-001: Parse valid Measured Height (Local Cartesian Coordinates)
- TC-020-110-002: Verify format compliance
- TC-020-110-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-140: Parse Time of Day (I020/140)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I020/140 (Time of Day) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Absolute time stamping expressed as UTC.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: mandatory
- Note: The time of day value is reset to zero each day at midnight.

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-140-001: Parse valid Time of Day
- TC-020-140-002: Verify format compliance
- TC-020-140-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-020-161: Parse Track Number (I020/161)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/161 (Track Number) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
An integer value representing a unique reference to a track record within a particular track file.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-161-001: Parse valid Track Number
- TC-020-161-002: Verify format compliance
- TC-020-161-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-170: Parse Track Status (I020/170)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/170 (Track Status) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Status of track.

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-170-001: Parse valid Track Status
- TC-020-170-002: Verify format compliance
- TC-020-170-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-202: Parse Calculated Track Velocity in Cartesian Coordinates (I020/202)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/202 (Calculated Track Velocity in Cartesian Coordinates) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Calculated track velocity expressed in Cartesian Coordinates, in two's complement representation.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-202-001: Parse valid Calculated Track Velocity in Cartesian Coordinates
- TC-020-202-002: Verify format compliance
- TC-020-202-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-210: Parse Calculated Acceleration (I020/210)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/210 (Calculated Acceleration) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Calculated Acceleration of the target, in two's complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-210-001: Parse valid Calculated Acceleration
- TC-020-210-002: Verify format compliance
- TC-020-210-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-220: Parse Target Address (I020/220)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/220 (Target Address) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Target address (24-bit address) assigned to each Target.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: The type of address (ICAO or non-ICAO address, type of message) is defined by the CF-indication in Data Item I020/020, second extension, bits 8/7.

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-220-001: Parse valid Target Address
- TC-020-220-002: Verify format compliance
- TC-020-220-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-230: Parse Communications/ACAS Capability and Flight Status (I020/230)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I020/230 (Communications/ACAS Capability and Flight Status) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Communications capability of the transponder, capability of the on-board ACAS equipment and flight status.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-230-001: Parse valid Communications/ACAS Capability and Flight Status
- TC-020-230-002: Verify format compliance
- TC-020-230-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-020-245: Parse Target Identification (I020/245)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/245 (Target Identification) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Target (aircraft or vehicle) identification in 8 characters.

**Implementation Notes:**
- Format: Fixed (7 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-245-001: Parse valid Target Identification
- TC-020-245-002: Verify format compliance
- TC-020-245-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-250: Parse Mode S MB Data (I020/250)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/250 (Mode S MB Data) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Mode S Comm B data as extracted from the aircraft transponder.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-250-001: Parse valid Mode S MB Data
- TC-020-250-002: Verify format compliance
- TC-020-250-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-260: Parse ACAS Resolution Advisory Report (I020/260)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/260 (ACAS Resolution Advisory Report) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Currently active Resolution Advisory (RA), if any, generated by the ACAS associated with the transponder transmitting the report and threat identity data.

**Implementation Notes:**
- Format: Fixed (7 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-260-001: Parse valid ACAS Resolution Advisory Report
- TC-020-260-002: Verify format compliance
- TC-020-260-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-300: Parse Vehicle Fleet Identification (I020/300)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/300 (Vehicle Fleet Identification) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Vehicle fleet identification number.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-300-001: Parse valid Vehicle Fleet Identification
- TC-020-300-002: Verify format compliance
- TC-020-300-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-310: Parse Pre-programmed Message (I020/310)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/310 (Pre-programmed Message) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Number related to a pre-programmed message that can be transmitted by a vehicle.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-310-001: Parse valid Pre-programmed Message
- TC-020-310-002: Verify format compliance
- TC-020-310-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-400: Parse Contributing Devices (I020/400)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/400 (Contributing Devices) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Overview of the devices, which have contributed to the Target Detection. In case of active Multilateration systems this may include transmitters.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-400-001: Parse valid Contributing Devices
- TC-020-400-002: Verify format compliance
- TC-020-400-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-500: Parse Position Accuracy (I020/500)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/500 (Position Accuracy) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Standard Deviation of Position

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-500-001: Parse valid Position Accuracy
- TC-020-500-002: Verify format compliance
- TC-020-500-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-RE: Parse Reserved Expansion Field (I020/RE)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/RE (Reserved Expansion Field) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
Reserved Expansion Field of ASTERIX Cat 020 (Multilateration Target Reports) v1.4

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-RE-001: Parse valid Reserved Expansion Field
- TC-020-RE-002: Verify format compliance
- TC-020-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-020-SP: Parse SP (I020/SP)

**Parent:** REQ-HLR-CAT-020
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I020/SP (SP) as specified in the EUROCONTROL ASTERIX CAT020 v1.10 specification.

**Data Item Definition:**
SP

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-SP-001: Parse valid SP
- TC-020-SP-002: Verify format compliance
- TC-020-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat020_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

## 3. Error Handling Requirements

### REQ-LLR-020-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT020 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-ERR-001: Truncated data items
- TC-020-ERR-002: Invalid repetition counts
- TC-020-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-020-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT020 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-020-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-020-010 | I020/010 | Data Source Identifier | Critical | High |
| REQ-LLR-020-020 | I020/020 | Target Report Descriptor | Critical | High |
| REQ-LLR-020-030 | I020/030 | Warning/Error Conditions | High | Medium |
| REQ-LLR-020-041 | I020/041 | Position in WGS-84 Coordinates | Critical | High |
| REQ-LLR-020-042 | I020/042 | Position in Cartesian Coordinates | High | Medium |
| REQ-LLR-020-050 | I020/050 | Mode-2 Code in Octal Representation | High | Medium |
| REQ-LLR-020-055 | I020/055 | Mode-1 Code in Octal Representation | High | Medium |
| REQ-LLR-020-070 | I020/070 | Mode-3/A Code in Octal Representation | High | Medium |
| REQ-LLR-020-090 | I020/090 | Flight Level in Binary Representation | High | Medium |
| REQ-LLR-020-100 | I020/100 | Mode-C Code | High | Medium |
| REQ-LLR-020-105 | I020/105 | Geometric Height (WGS-84) | High | Medium |
| REQ-LLR-020-110 | I020/110 | Measured Height (Local Cartesian Coordin | High | Medium |
| REQ-LLR-020-140 | I020/140 | Time of Day | Critical | High |
| REQ-LLR-020-161 | I020/161 | Track Number | High | Medium |
| REQ-LLR-020-170 | I020/170 | Track Status | High | Medium |
| REQ-LLR-020-202 | I020/202 | Calculated Track Velocity in Cartesian C | High | Medium |
| REQ-LLR-020-210 | I020/210 | Calculated Acceleration | High | Medium |
| REQ-LLR-020-220 | I020/220 | Target Address | High | Medium |
| REQ-LLR-020-230 | I020/230 | Communications/ACAS Capability and Fligh | Critical | High |
| REQ-LLR-020-245 | I020/245 | Target Identification | High | Medium |
| REQ-LLR-020-250 | I020/250 | Mode S MB Data | High | Medium |
| REQ-LLR-020-260 | I020/260 | ACAS Resolution Advisory Report | High | Medium |
| REQ-LLR-020-300 | I020/300 | Vehicle Fleet Identification | High | Medium |
| REQ-LLR-020-310 | I020/310 | Pre-programmed Message | High | Medium |
| REQ-LLR-020-400 | I020/400 | Contributing Devices | High | Medium |
| REQ-LLR-020-500 | I020/500 | Position Accuracy | High | Medium |
| REQ-LLR-020-RE | I020/RE | Reserved Expansion Field | High | Medium |
| REQ-LLR-020-SP | I020/SP | SP | High | Medium |
| REQ-LLR-020-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-020-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 30

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-020
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-020-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT020 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
