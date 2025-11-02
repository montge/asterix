# Low-Level Requirements - ASTERIX Category 11
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT11-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-11
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 11 (Transmission of A-SMGCS Data) v1.2.

**Parent Requirement:** REQ-HLR-CAT-11 - Parse ASTERIX Category 11

Each data item in CAT11 has corresponding LLRs defining parsing behavior.

---

## 2. CAT11 Data Item Requirements

### REQ-LLR-11-000: Parse Message Type (I11/000)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/000 (Message Type) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            This Data Item allows for a more convenient handling of the messages
            at the receiver side by further defining the type of transaction.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-000-001: Parse valid Message Type
- TC-11-000-002: Verify format compliance
- TC-11-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-010: Parse Data Source Identifier (I11/010)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Identification of the radar station from which the data are received.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-010-001: Parse valid Data Source Identifier
- TC-11-010-002: Verify format compliance
- TC-11-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-015: Parse Service Identification (I11/015)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/015 (Service Identification) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Identification of the service provided to one or more users.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-015-001: Parse valid Service Identification
- TC-11-015-002: Verify format compliance
- TC-11-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-041: Parse Position in WGS-84 Coordinates (I11/041)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/041 (Position in WGS-84 Coordinates) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Position of a target in WGS-84 Coordinates.
        

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-041-001: Parse valid Position in WGS-84 Coordinates
- TC-11-041-002: Verify format compliance
- TC-11-041-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-042: Parse Calculated Position in Cartesian Co-ordinates (I11/042)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/042 (Calculated Position in Cartesian Co-ordinates) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Calculated position of a target in Cartesian co-ordinates (two's complement form).
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-042-001: Parse valid Calculated Position in Cartesian Co-ordinates
- TC-11-042-002: Verify format compliance
- TC-11-042-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-060: Parse Mode-3/A Code in Octal Representation (I11/060)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/060 (Mode-3/A Code in Octal Representation) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Track Mode-3/A code converted into octal representation.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-060-001: Parse valid Mode-3/A Code in Octal Representation
- TC-11-060-002: Verify format compliance
- TC-11-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-090: Parse Measured Flight Level (I11/090)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/090 (Measured Flight Level) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Last valid and credible flight level used to update the track, in two's complement representation.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-090-001: Parse valid Measured Flight Level
- TC-11-090-002: Verify format compliance
- TC-11-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-092: Parse Calculated Track Geometric Altitude (I11/092)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/092 (Calculated Track Geometric Altitude) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Calculated Track Geometric Altitude Calculated geometric vertical distance above mean sea level, not related to barometric pressure.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-092-001: Parse valid Calculated Track Geometric Altitude
- TC-11-092-002: Verify format compliance
- TC-11-092-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-093: Parse Calculated Track Barometric Altitude (I11/093)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/093 (Calculated Track Barometric Altitude) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Calculated Barometric Altitude of the track.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-093-001: Parse valid Calculated Track Barometric Altitude
- TC-11-093-002: Verify format compliance
- TC-11-093-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-140: Parse Time of Track Information (I11/140)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/140 (Time of Track Information) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Absolute time stamping expressed as UTC.
        

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-140-001: Parse valid Time of Track Information
- TC-11-140-002: Verify format compliance
- TC-11-140-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-161: Parse Track Number (I11/161)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/161 (Track Number) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Identification of a fusion track (single track number).
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-161-001: Parse valid Track Number
- TC-11-161-002: Verify format compliance
- TC-11-161-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-170: Parse Track Status (I11/170)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/170 (Track Status) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Status of track.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-170-001: Parse valid Track Status
- TC-11-170-002: Verify format compliance
- TC-11-170-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-202: Parse Calculated Track Velocity in Cartesian Coordinates (I11/202)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/202 (Calculated Track Velocity in Cartesian Coordinates) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Calculated track velocity expressed in Cartesian co-ordinates.
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-202-001: Parse valid Calculated Track Velocity in Cartesian Coordinates
- TC-11-202-002: Verify format compliance
- TC-11-202-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-210: Parse Calculated Acceleration (I11/210)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/210 (Calculated Acceleration) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Calculated Acceleration of the target, in two's complement form.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-210-001: Parse valid Calculated Acceleration
- TC-11-210-002: Verify format compliance
- TC-11-210-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-215: Parse Calculated Rate Of Climb/Descent (I11/215)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/215 (Calculated Rate Of Climb/Descent) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Calculated rate of Climb/Descent of an aircraft, in two's complement form.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-215-001: Parse valid Calculated Rate Of Climb/Descent
- TC-11-215-002: Verify format compliance
- TC-11-215-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-245: Parse Target Identification (I11/245)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/245 (Target Identification) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Target (aircraft or vehicle) identification in 8 characters.
        

**Implementation Notes:**
- Format: Fixed (7 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-245-001: Parse valid Target Identification
- TC-11-245-002: Verify format compliance
- TC-11-245-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-270: Parse Target Size and Orientation (I11/270)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/270 (Target Size and Orientation) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Target size defined as length and with of the detected target, and orientation.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-270-001: Parse valid Target Size and Orientation
- TC-11-270-002: Verify format compliance
- TC-11-270-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-290: Parse System Track Update Ages (I11/290)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/290 (System Track Update Ages) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Ages of the last plot/local track, or the last valid mode-A/mode-C, used to update the system track.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-290-001: Parse valid System Track Update Ages
- TC-11-290-002: Verify format compliance
- TC-11-290-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-300: Parse Vehicle Fleet Identification (I11/300)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/300 (Vehicle Fleet Identification) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Vehicle fleet identification number.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-300-001: Parse valid Vehicle Fleet Identification
- TC-11-300-002: Verify format compliance
- TC-11-300-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-310: Parse Pre-programmed Message (I11/310)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/310 (Pre-programmed Message) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Number related to a pre-programmed message that can be transmitted by a vehicle.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-310-001: Parse valid Pre-programmed Message
- TC-11-310-002: Verify format compliance
- TC-11-310-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-380: Parse Mode-S / ADS-B Related Data (I11/380)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/380 (Mode-S / ADS-B Related Data) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Data specific to Mode-S ADS-B.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-380-001: Parse valid Mode-S / ADS-B Related Data
- TC-11-380-002: Verify format compliance
- TC-11-380-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-390: Parse Flight Plan Related Data (I11/390)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/390 (Flight Plan Related Data) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            All flight plan related information.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-390-001: Parse valid Flight Plan Related Data
- TC-11-390-002: Verify format compliance
- TC-11-390-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-430: Parse Phase of flight (I11/430)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/430 (Phase of flight) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Current phase of the flight.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-430-001: Parse valid Phase of flight
- TC-11-430-002: Verify format compliance
- TC-11-430-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-500: Parse Estimated Accuracies (I11/500)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/500 (Estimated Accuracies) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Overview of all important accuracies (standard deviations).
        

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-500-001: Parse valid Estimated Accuracies
- TC-11-500-002: Verify format compliance
- TC-11-500-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-600: Parse Alert messages (I11/600)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/600 (Alert messages) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Alert involving the targets indicated in I011/605.
        

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-600-001: Parse valid Alert messages
- TC-11-600-002: Verify format compliance
- TC-11-600-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-605: Parse Tracks in Alert (I11/605)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/605 (Tracks in Alert) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            List of track numbers of the targets concerned by the alert described in I011/600.
        

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-605-001: Parse valid Tracks in Alert
- TC-11-605-002: Verify format compliance
- TC-11-605-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-610: Parse Holdbar status (I11/610)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/610 (Holdbar status) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            LStatus of up to sixteen banks of twelve indicators.
        

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-610-001: Parse valid Holdbar status
- TC-11-610-002: Verify format compliance
- TC-11-610-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-SP: Parse Special Purpose Field (I11/SP)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Special Purpose Field
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-SP-001: Parse valid Special Purpose Field
- TC-11-SP-002: Verify format compliance
- TC-11-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-11-RE: Parse Reserved Expansion Field (I11/RE)

**Parent:** REQ-HLR-CAT-11
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I11/RE (Reserved Expansion Field) as specified in the EUROCONTROL ASTERIX CAT11 v1.2 specification.

**Data Item Definition:**

            Expansion
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-RE-001: Parse valid Reserved Expansion Field
- TC-11-RE-002: Verify format compliance
- TC-11-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-11-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT11 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-ERR-001: Truncated data items
- TC-11-ERR-002: Invalid repetition counts
- TC-11-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-11-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT11 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-11-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-11-000 | I11/000 | Message Type | Medium | Low |
| REQ-LLR-11-010 | I11/010 | Data Source Identifier | Medium | Low |
| REQ-LLR-11-015 | I11/015 | Service Identification | Medium | Low |
| REQ-LLR-11-041 | I11/041 | Position in WGS-84 Coordinates | Medium | Low |
| REQ-LLR-11-042 | I11/042 | Calculated Position in Cartesian Co-ordi | Medium | Low |
| REQ-LLR-11-060 | I11/060 | Mode-3/A Code in Octal Representation | Medium | Low |
| REQ-LLR-11-090 | I11/090 | Measured Flight Level | Medium | Low |
| REQ-LLR-11-092 | I11/092 | Calculated Track Geometric Altitude | Medium | Low |
| REQ-LLR-11-093 | I11/093 | Calculated Track Barometric Altitude | Medium | Low |
| REQ-LLR-11-140 | I11/140 | Time of Track Information | Medium | Low |
| REQ-LLR-11-161 | I11/161 | Track Number | Medium | Low |
| REQ-LLR-11-170 | I11/170 | Track Status | Medium | Low |
| REQ-LLR-11-202 | I11/202 | Calculated Track Velocity in Cartesian C | Medium | Low |
| REQ-LLR-11-210 | I11/210 | Calculated Acceleration | Medium | Low |
| REQ-LLR-11-215 | I11/215 | Calculated Rate Of Climb/Descent | Medium | Low |
| REQ-LLR-11-245 | I11/245 | Target Identification | Medium | Low |
| REQ-LLR-11-270 | I11/270 | Target Size and Orientation | Medium | Low |
| REQ-LLR-11-290 | I11/290 | System Track Update Ages | Medium | Low |
| REQ-LLR-11-300 | I11/300 | Vehicle Fleet Identification | Medium | Low |
| REQ-LLR-11-310 | I11/310 | Pre-programmed Message | Medium | Low |
| REQ-LLR-11-380 | I11/380 | Mode-S / ADS-B Related Data | Medium | Low |
| REQ-LLR-11-390 | I11/390 | Flight Plan Related Data | Medium | Low |
| REQ-LLR-11-430 | I11/430 | Phase of flight | Medium | Low |
| REQ-LLR-11-500 | I11/500 | Estimated Accuracies | Medium | Low |
| REQ-LLR-11-600 | I11/600 | Alert messages | Medium | Low |
| REQ-LLR-11-605 | I11/605 | Tracks in Alert | Medium | Low |
| REQ-LLR-11-610 | I11/610 | Holdbar status | Medium | Low |
| REQ-LLR-11-SP | I11/SP | Special Purpose Field | Medium | Low |
| REQ-LLR-11-RE | I11/RE | Reserved Expansion Field | Medium | Low |
| REQ-LLR-11-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-11-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 31

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-11
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-11-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT11 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
