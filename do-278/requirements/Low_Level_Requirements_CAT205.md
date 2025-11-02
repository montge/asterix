# Low-Level Requirements - ASTERIX Category 205
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT205-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-205
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 205 (Radio Direction Finder Reports) v1.0.

**Parent Requirement:** REQ-HLR-CAT-205 - Parse ASTERIX Category 205

Each data item in CAT205 has corresponding LLRs defining parsing behavior.

---

## 2. CAT205 Data Item Requirements

### REQ-LLR-205-000: Parse Message Type (I205/000)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/000 (Message Type) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            This Data Item allows for a more convenient handling of the
            messages at the receiver side by further defining the type of
            transaction.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-000-001: Parse valid Message Type
- TC-205-000-002: Verify format compliance
- TC-205-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-010: Parse Data Source Identifier (I205/010)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Identification of the Radio Direction Finder System or Sensor from which the
            report is received.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The up-to-date list of SACs is published on the EUROCONTROL Web Site (http://www.eurocontrol.int/asterix).
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-010-001: Parse valid Data Source Identifier
- TC-205-010-002: Verify format compliance
- TC-205-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-015: Parse Service Identification (I205/015)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/015 (Service Identification) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Identification of the service provided to one or more users.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: The service identification is allocated by the system.

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-015-001: Parse valid Service Identification
- TC-205-015-002: Verify format compliance
- TC-205-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-030: Parse Time of Day (I205/030)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/030 (Time of Day) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            UTC time of transmission of this ASTERIX message.
        

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: The TOD value is reset to zero at every midnight UTC.

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-030-001: Parse valid Time of Day
- TC-205-030-002: Verify format compliance
- TC-205-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-040: Parse Report Number (I205/040)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/040 (Report Number) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Sequential and cyclic number of position detection reports.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
                The number is incremented if there is a new position or bearing detected, independent
                from the respective channel. The report number shall also be incremented if, during
                the ongoing position or bearing detection, the position or bearing value is deviating by
                a system-defined threshold from the previous one. If the message type (I205/000)
                changes to another Detection Report, the report number is incremented as well.
                to detect lost messages.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-040-001: Parse valid Report Number
- TC-205-040-002: Verify format compliance
- TC-205-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-050: Parse Position in WGS-84 Coordinates (I205/050)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/050 (Position in WGS-84 Coordinates) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Calculated position in WGS-84 Coordinates.
        

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: optional
- Note: 
                The LSB provides a resolution at least better than 0.6 meters.
                For reports of Message Type 001 and 003, the item shall contain the estimated
                position of the transmitting aircraft.
                For reports of Message Type 002, the item shall contain the position of the bearing
                starting point, i.e. the position of the respective RDF sensor.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-050-001: Parse valid Position in WGS-84 Coordinates
- TC-205-050-002: Verify format compliance
- TC-205-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-060: Parse Position in Cartesian Coordinates (I205/060)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/060 (Position in Cartesian Coordinates) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Calculated position in Cartesian Coordinates.
        

**Implementation Notes:**
- Format: Fixed (6 bytes)
- Rule: optional
- Note: 
                The Cartesian coordinates are relative to an agreed System Reference Point. The
                System Reference Point may be communicated in ASTERIX category 025, item 600.
                For reports of Message Type 001 and 003, the item shall contain the estimated
                position of the transmitting aircraft.
                For reports of Message Type 002, the item shall contain the position of the bearing
                starting point, i.e. the position of the respective RDF sensor.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-060-001: Parse valid Position in Cartesian Coordinates
- TC-205-060-002: Verify format compliance
- TC-205-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-070: Parse Local Bearing (I205/070)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/070 (Local Bearing) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Bearing of the detected radio transmission, starting at the position contained in
            I205/050 and given relative to this position.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
                The angle is given in degrees, in clock-wise notation,
                starting with 0 degrees for the geographical North.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-070-001: Parse valid Local Bearing
- TC-205-070-002: Verify format compliance
- TC-205-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-080: Parse System Bearing (I205/080)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/080 (System Bearing) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Bearing of the detected radio transmission, starting at the position contained in
            I205/060, projected to the Cartesian Coordinate System relative to the System
            Reference Point (as used for I205/ 060).
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
                The angle is given in degrees, in clock-wise notation,
                starting with 0 degrees for the geographical North.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-080-001: Parse valid System Bearing
- TC-205-080-002: Verify format compliance
- TC-205-080-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-090: Parse Radio Channel Name (I205/090)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/090 (Radio Channel Name) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Name of the channel the radio transmission is detected on.
        

**Implementation Notes:**
- Format: Fixed (7 bytes)
- Rule: optional
- Note: 
                NU1 till NU7 contain digits or a decimal point in ASCII representation, specifying the
                name of the radio channel. Channel names that could be provided with less than 6
                digits shall be filled with trailing zeroes (e.g. 121.100).
                This channel name is not identical with the actual physical frequency.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-090-001: Parse valid Radio Channel Name
- TC-205-090-002: Verify format compliance
- TC-205-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-100: Parse Quality of Measurement (I205/100)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/100 (Quality of Measurement) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Quality of the measurement provided by the Radio Direction Finder system.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
                The actual meanings of the bits are application dependent.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-100-001: Parse valid Quality of Measurement
- TC-205-100-002: Verify format compliance
- TC-205-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-110: Parse Estimated Uncertainty (I205/110)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/110 (Estimated Uncertainty) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Uncertainty estimation of the RDF System. The transmitter is expected to be
            within the provided radius around the calculated position.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-110-001: Parse valid Estimated Uncertainty
- TC-205-110-002: Verify format compliance
- TC-205-110-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-120: Parse Contributing Sensors (I205/120)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/120 (Contributing Sensors) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            The identification of the RDF sensors that contributed to the detection of the radio
            transmitter.
        

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: 
                The actual identification of the receivers is application dependent.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-120-001: Parse valid Contributing Sensors
- TC-205-120-002: Verify format compliance
- TC-205-120-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-130: Parse Conflicting Transmitter Position in WGS-84 Coordinates (I205/130)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/130 (Conflicting Transmitter Position in WGS-84 Coordinates) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Calculated position in WGS-84 Coordinates. This is the position of a second
            transmitter on the same frequency and overlapping in time with the transmitter
            position communicated in data item I205/050.
        

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: optional
- Note: 
                The LSB provides a resolution at least better than 0.6 meters.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-130-001: Parse valid Conflicting Transmitter Position in WGS-84 Coordinates
- TC-205-130-002: Verify format compliance
- TC-205-130-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-140: Parse Conflicting Transmitter Position in Cartesian Coordinates (I205/140)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/140 (Conflicting Transmitter Position in Cartesian Coordinates) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Calculated position in Cartesian Coordinates. This is the position of a second
            transmitter on the same frequency and overlapping in time with the transmitter
            position communicated in data item I205/060.
        

**Implementation Notes:**
- Format: Fixed (6 bytes)
- Rule: optional
- Note: 
                The Cartesian coordinates are relative to an agreed System Reference Point. The
                System Reference Point may be communicated in ASTERIX category 025, item 600.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-140-001: Parse valid Conflicting Transmitter Position in Cartesian Coordinates
- TC-205-140-002: Verify format compliance
- TC-205-140-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-150: Parse Conflicting Transmitter Estimated Uncertainty (I205/150)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/150 (Conflicting Transmitter Estimated Uncertainty) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Range uncertainty estimation of the RDF System for the Conflicting Transmitter,
            i.e. a transmitter on the same frequency and with a timely overlapping
            transmission. The transmitter is estimated to be within the provided radius around
            the detected position.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-150-001: Parse valid Conflicting Transmitter Estimated Uncertainty
- TC-205-150-002: Verify format compliance
- TC-205-150-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-160: Parse Track Number (I205/160)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/160 (Track Number) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Unique identification of a track at the calculated RDF position.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-160-001: Parse valid Track Number
- TC-205-160-002: Verify format compliance
- TC-205-160-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-170: Parse Sensor Identification (I205/170)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/170 (Sensor Identification) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Unique identification of an RDF sensor.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
                For Message Type 5 (Sensor Data Report) in I205/000, the item has to be used if
                there is no unique SAC/SIC defined for each RDF Sensor.
                The actual identification number is application dependent
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-170-001: Parse valid Sensor Identification
- TC-205-170-002: Verify format compliance
- TC-205-170-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-180: Parse Signal Level (I205/180)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/180 (Signal Level) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            The level of the signal received by an RDF sensor.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-180-001: Parse valid Signal Level
- TC-205-180-002: Verify format compliance
- TC-205-180-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-190: Parse Signal Quality (I205/190)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/190 (Signal Quality) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Relative quality of the received signal as estimated by the RDF sensor.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
                255 corresponds to the best quality,
                0 to the worst quality
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-190-001: Parse valid Signal Quality
- TC-205-190-002: Verify format compliance
- TC-205-190-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-200: Parse Signal Elevation (I205/200)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/200 (Signal Elevation) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            The elevation of the signal received by an RDF sensor.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-200-001: Parse valid Signal Elevation
- TC-205-200-002: Verify format compliance
- TC-205-200-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-205-SP: Parse Special Purpose Field (I205/SP)

**Parent:** REQ-HLR-CAT-205
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I205/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT205 v1.0 specification.

**Data Item Definition:**

            Special Purpose Field
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-SP-001: Parse valid Special Purpose Field
- TC-205-SP-002: Verify format compliance
- TC-205-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat205_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-205-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT205 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-ERR-001: Truncated data items
- TC-205-ERR-002: Invalid repetition counts
- TC-205-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-205-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT205 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-205-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-205-000 | I205/000 | Message Type | Medium | Low |
| REQ-LLR-205-010 | I205/010 | Data Source Identifier | Medium | Low |
| REQ-LLR-205-015 | I205/015 | Service Identification | Medium | Low |
| REQ-LLR-205-030 | I205/030 | Time of Day | Medium | Low |
| REQ-LLR-205-040 | I205/040 | Report Number | Medium | Low |
| REQ-LLR-205-050 | I205/050 | Position in WGS-84 Coordinates | Medium | Low |
| REQ-LLR-205-060 | I205/060 | Position in Cartesian Coordinates | Medium | Low |
| REQ-LLR-205-070 | I205/070 | Local Bearing | Medium | Low |
| REQ-LLR-205-080 | I205/080 | System Bearing | Medium | Low |
| REQ-LLR-205-090 | I205/090 | Radio Channel Name | Medium | Low |
| REQ-LLR-205-100 | I205/100 | Quality of Measurement | Medium | Low |
| REQ-LLR-205-110 | I205/110 | Estimated Uncertainty | Medium | Low |
| REQ-LLR-205-120 | I205/120 | Contributing Sensors | Medium | Low |
| REQ-LLR-205-130 | I205/130 | Conflicting Transmitter Position in WGS- | Medium | Low |
| REQ-LLR-205-140 | I205/140 | Conflicting Transmitter Position in Cart | Medium | Low |
| REQ-LLR-205-150 | I205/150 | Conflicting Transmitter Estimated Uncert | Medium | Low |
| REQ-LLR-205-160 | I205/160 | Track Number | Medium | Low |
| REQ-LLR-205-170 | I205/170 | Sensor Identification | Medium | Low |
| REQ-LLR-205-180 | I205/180 | Signal Level | Medium | Low |
| REQ-LLR-205-190 | I205/190 | Signal Quality | Medium | Low |
| REQ-LLR-205-200 | I205/200 | Signal Elevation | Medium | Low |
| REQ-LLR-205-SP | I205/SP | Special Purpose Field | Medium | Low |
| REQ-LLR-205-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-205-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 24

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-205
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-205-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT205 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
