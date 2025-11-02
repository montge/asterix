# Low-Level Requirements - ASTERIX Category 011
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT011-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-011
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 011 (Transmission of A-SMGCS Data) v1.3.

**Parent Requirement:** REQ-HLR-CAT-011 - Parse ASTERIX Category 011

Each data item in CAT011 has corresponding LLRs defining parsing behavior.

---

## 2. CAT011 Data Item Requirements

### REQ-LLR-011-000: Parse Message Type (I011/000)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/000 (Message Type) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            This Data Item allows for a more convenient handling of the messages
            at the receiver side by further defining the type of transaction.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-000-001: Parse valid Message Type
- TC-011-000-002: Verify format compliance
- TC-011-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-010: Parse Data Source Identifier (I011/010)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Identification of the radar station from which the data are received.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Note:
                The SAC is fixed to zero to indicate a data flow local to the airport.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-010-001: Parse valid Data Source Identifier
- TC-011-010-002: Verify format compliance
- TC-011-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-015: Parse Service Identification (I011/015)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/015 (Service Identification) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Identification of the service provided to one or more users.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
            Note:
                The service identification is allocated by the A-SMGCS
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-015-001: Parse valid Service Identification
- TC-011-015-002: Verify format compliance
- TC-011-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-041: Parse Position in WGS-84 Coordinates (I011/041)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/041 (Position in WGS-84 Coordinates) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Position of a target in WGS-84 Coordinates.
        

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-041-001: Parse valid Position in WGS-84 Coordinates
- TC-011-041-002: Verify format compliance
- TC-011-041-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-042: Parse Calculated Position in Cartesian Co-ordinates (I011/042)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/042 (Calculated Position in Cartesian Co-ordinates) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Calculated position of a target in Cartesian co-ordinates (two's complement form).
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-042-001: Parse valid Calculated Position in Cartesian Co-ordinates
- TC-011-042-002: Verify format compliance
- TC-011-042-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-060: Parse Mode-3/A Code in Octal Representation (I011/060)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/060 (Mode-3/A Code in Octal Representation) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Track Mode-3/A code converted into Octal Representation.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-060-001: Parse valid Mode-3/A Code in Octal Representation
- TC-011-060-002: Verify format compliance
- TC-011-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-090: Parse Measured Flight Level (I011/090)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/090 (Measured Flight Level) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Last valid and credible flight level used to update the track, in two's complement representation.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Note:
                 The criteria to determine the credibility of the flight level are Tracker dependent.
                 Credible means: within reasonable range of change with respect to the previous detection.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-090-001: Parse valid Measured Flight Level
- TC-011-090-002: Verify format compliance
- TC-011-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-092: Parse Calculated Track Geometric Altitude (I011/092)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/092 (Calculated Track Geometric Altitude) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Calculated geometric vertical distance above mean sea level, not related to barometric pressure.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Note:
                 The source of altitude is identified in bits (SRC) of item I011/170 Track Status.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-092-001: Parse valid Calculated Track Geometric Altitude
- TC-011-092-002: Verify format compliance
- TC-011-092-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-093: Parse Calculated Track Barometric Altitude (I011/093)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/093 (Calculated Track Barometric Altitude) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Calculated Barometric Altitude of the track.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-093-001: Parse valid Calculated Track Barometric Altitude
- TC-011-093-002: Verify format compliance
- TC-011-093-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-140: Parse Time of Track Information (I011/140)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/140 (Time of Track Information) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Absolute time stamping expressed as UTC.
        

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: 
            Note:
                The Time of Track Information value is reset to zero each day at midnight.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-140-001: Parse valid Time of Track Information
- TC-011-140-002: Verify format compliance
- TC-011-140-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-161: Parse Track Number (I011/161)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/161 (Track Number) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Identification of a fusion track (single track number).
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-161-001: Parse valid Track Number
- TC-011-161-002: Verify format compliance
- TC-011-161-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-170: Parse Track Status (I011/170)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/170 (Track Status) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Status of track.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            Track type and coasting can also be derived from Data Item I011/290 System Track Update Ages
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-170-001: Parse valid Track Status
- TC-011-170-002: Verify format compliance
- TC-011-170-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-202: Parse Calculated Track Velocity in Cartesian Coordinates (I011/202)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/202 (Calculated Track Velocity in Cartesian Coordinates) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Calculated track velocity expressed in Cartesian co-ordinates.
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-202-001: Parse valid Calculated Track Velocity in Cartesian Coordinates
- TC-011-202-002: Verify format compliance
- TC-011-202-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-210: Parse Calculated Acceleration (I011/210)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/210 (Calculated Acceleration) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Calculated Acceleration of the target, in two's complement form.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-210-001: Parse valid Calculated Acceleration
- TC-011-210-002: Verify format compliance
- TC-011-210-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-215: Parse Calculated Rate Of Climb/Descent (I011/215)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/215 (Calculated Rate Of Climb/Descent) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Calculated rate of Climb/Descent of an aircraft, in two's complement form.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-215-001: Parse valid Calculated Rate Of Climb/Descent
- TC-011-215-002: Verify format compliance
- TC-011-215-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-245: Parse Target Identification (I011/245)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/245 (Target Identification) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Target (aircraft or vehicle) identification in 8 characters.
        

**Implementation Notes:**
- Format: Fixed (7 bytes)
- Rule: optional
- Note: 
            Note:
                Characters 1-8 (coded on 6 bits each) defining target identification
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-245-001: Parse valid Target Identification
- TC-011-245-002: Verify format compliance
- TC-011-245-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-270: Parse Target Size and Orientation (I011/270)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/270 (Target Size and Orientation) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Target size defined as length and with of the detected target, and orientation.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            Note:
                The orientation gives the direction to which the aircraft nose is pointing, relative to the Geographical North.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-270-001: Parse valid Target Size and Orientation
- TC-011-270-002: Verify format compliance
- TC-011-270-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-290: Parse System Track Update Ages (I011/290)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/290 (System Track Update Ages) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Ages of the last plot/local track, or the last valid mode-A/mode-C, used to update the system track.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional
- Note: 
            Note:
                The ages are counted from Data Item I011/140, Time Of Track
                Information, using the following formula:
                Age = Time of track information - Time of last (valid) update
                If the computed age is greater than the maximum value or if the
                data has never been received, then the corresponding subfield is not sent.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-290-001: Parse valid System Track Update Ages
- TC-011-290-002: Verify format compliance
- TC-011-290-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-300: Parse Vehicle Fleet Identification (I011/300)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/300 (Vehicle Fleet Identification) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Vehicle fleet identification number.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-300-001: Parse valid Vehicle Fleet Identification
- TC-011-300-002: Verify format compliance
- TC-011-300-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-310: Parse Pre-programmed Message (I011/310)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/310 (Pre-programmed Message) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Number related to a pre-programmed message that can be transmitted by a vehicle.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-310-001: Parse valid Pre-programmed Message
- TC-011-310-002: Verify format compliance
- TC-011-310-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-380: Parse Mode-S / ADS-B Related Data (I011/380)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/380 (Mode-S / ADS-B Related Data) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Data specific to Mode-S ADS-B.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-380-001: Parse valid Mode-S / ADS-B Related Data
- TC-011-380-002: Verify format compliance
- TC-011-380-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-390: Parse Flight Plan Related Data (I011/390)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/390 (Flight Plan Related Data) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            All flight plan related information.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-390-001: Parse valid Flight Plan Related Data
- TC-011-390-002: Verify format compliance
- TC-011-390-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-430: Parse Phase of Flight (I011/430)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/430 (Phase of Flight) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Current phase of the flight.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-430-001: Parse valid Phase of Flight
- TC-011-430-002: Verify format compliance
- TC-011-430-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-500: Parse Estimated Accuracies (I011/500)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/500 (Estimated Accuracies) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Overview of all important accuracies (standard deviations).
        

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-500-001: Parse valid Estimated Accuracies
- TC-011-500-002: Verify format compliance
- TC-011-500-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-600: Parse Alert Messages (I011/600)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/600 (Alert Messages) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Alert involving the targets indicated in I011/605.
        

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-600-001: Parse valid Alert Messages
- TC-011-600-002: Verify format compliance
- TC-011-600-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-605: Parse Tracks in Alert (I011/605)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/605 (Tracks in Alert) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            List of track numbers of the targets concerned by the alert described in I011/600.
        

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-605-001: Parse valid Tracks in Alert
- TC-011-605-002: Verify format compliance
- TC-011-605-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-610: Parse Holdbar Status (I011/610)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/610 (Holdbar Status) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Status of up to sixteen banks of twelve indicators.
        

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-610-001: Parse valid Holdbar Status
- TC-011-610-002: Verify format compliance
- TC-011-610-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-SP: Parse Special Purpose Field (I011/SP)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Special Purpose Field
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-SP-001: Parse valid Special Purpose Field
- TC-011-SP-002: Verify format compliance
- TC-011-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-011-RE: Parse Reserved Expansion Field (I011/RE)

**Parent:** REQ-HLR-CAT-011
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I011/RE (Reserved Expansion Field) as specified in the EUROCONTROL ASTERIX CAT011 v1.3 specification.

**Data Item Definition:**

            Expansion
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-RE-001: Parse valid Reserved Expansion Field
- TC-011-RE-002: Verify format compliance
- TC-011-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat011_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-011-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT011 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-ERR-001: Truncated data items
- TC-011-ERR-002: Invalid repetition counts
- TC-011-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-011-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT011 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-011-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-011-000 | I011/000 | Message Type | Medium | Low |
| REQ-LLR-011-010 | I011/010 | Data Source Identifier | Medium | Low |
| REQ-LLR-011-015 | I011/015 | Service Identification | Medium | Low |
| REQ-LLR-011-041 | I011/041 | Position in WGS-84 Coordinates | Medium | Low |
| REQ-LLR-011-042 | I011/042 | Calculated Position in Cartesian Co-ordi | Medium | Low |
| REQ-LLR-011-060 | I011/060 | Mode-3/A Code in Octal Representation | Medium | Low |
| REQ-LLR-011-090 | I011/090 | Measured Flight Level | Medium | Low |
| REQ-LLR-011-092 | I011/092 | Calculated Track Geometric Altitude | Medium | Low |
| REQ-LLR-011-093 | I011/093 | Calculated Track Barometric Altitude | Medium | Low |
| REQ-LLR-011-140 | I011/140 | Time of Track Information | Medium | Low |
| REQ-LLR-011-161 | I011/161 | Track Number | Medium | Low |
| REQ-LLR-011-170 | I011/170 | Track Status | Medium | Low |
| REQ-LLR-011-202 | I011/202 | Calculated Track Velocity in Cartesian C | Medium | Low |
| REQ-LLR-011-210 | I011/210 | Calculated Acceleration | Medium | Low |
| REQ-LLR-011-215 | I011/215 | Calculated Rate Of Climb/Descent | Medium | Low |
| REQ-LLR-011-245 | I011/245 | Target Identification | Medium | Low |
| REQ-LLR-011-270 | I011/270 | Target Size and Orientation | Medium | Low |
| REQ-LLR-011-290 | I011/290 | System Track Update Ages | Medium | Low |
| REQ-LLR-011-300 | I011/300 | Vehicle Fleet Identification | Medium | Low |
| REQ-LLR-011-310 | I011/310 | Pre-programmed Message | Medium | Low |
| REQ-LLR-011-380 | I011/380 | Mode-S / ADS-B Related Data | Medium | Low |
| REQ-LLR-011-390 | I011/390 | Flight Plan Related Data | Medium | Low |
| REQ-LLR-011-430 | I011/430 | Phase of Flight | Medium | Low |
| REQ-LLR-011-500 | I011/500 | Estimated Accuracies | Medium | Low |
| REQ-LLR-011-600 | I011/600 | Alert Messages | Medium | Low |
| REQ-LLR-011-605 | I011/605 | Tracks in Alert | Medium | Low |
| REQ-LLR-011-610 | I011/610 | Holdbar Status | Medium | Low |
| REQ-LLR-011-SP | I011/SP | Special Purpose Field | Medium | Low |
| REQ-LLR-011-RE | I011/RE | Reserved Expansion Field | Medium | Low |
| REQ-LLR-011-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-011-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 31

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-011
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-011-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT011 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
