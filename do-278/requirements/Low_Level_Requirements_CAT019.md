# Low-Level Requirements - ASTERIX Category 019
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT019-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-019
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 019 (Multilateration System Status Messages) v1.3.

**Parent Requirement:** REQ-HLR-CAT-019 - Parse ASTERIX Category 019

Each data item in CAT019 has corresponding LLRs defining parsing behavior.

---

## 2. CAT019 Data Item Requirements

### REQ-LLR-019-000: Parse Message Type (I019/000)

**Parent:** REQ-HLR-CAT-019
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I019/000 (Message Type) as specified in the EUROCONTROL ASTERIX CAT019 v1.3 specification.

**Data Item Definition:**
This Data Item allows for a more convenient handling of the messages at the receiver side by further defining the type of information.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-019-000-001: Parse valid Message Type
- TC-019-000-002: Verify format compliance
- TC-019-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat019_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-019-010: Parse Data Source Identifier (I019/010)

**Parent:** REQ-HLR-CAT-019
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I019/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT019 v1.3 specification.

**Data Item Definition:**
Identification of the system from which the data is received.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-019-010-001: Parse valid Data Source Identifier
- TC-019-010-002: Verify format compliance
- TC-019-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat019_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-019-140: Parse Time of Day (I019/140)

**Parent:** REQ-HLR-CAT-019
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I019/140 (Time of Day) as specified in the EUROCONTROL ASTERIX CAT019 v1.3 specification.

**Data Item Definition:**
Absolute time stamping expressed as UTC.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: mandatory
- Note: The time of day value is reset to zero each day at midnight

**Verification Method:** Unit Test
**Test Cases:**
- TC-019-140-001: Parse valid Time of Day
- TC-019-140-002: Verify format compliance
- TC-019-140-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat019_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-019-550: Parse System Status (I019/550)

**Parent:** REQ-HLR-CAT-019
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I019/550 (System Status) as specified in the EUROCONTROL ASTERIX CAT019 v1.3 specification.

**Data Item Definition:**
Information concerning the configuration and status of a System.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: A time source is considered as valid when either externally synchronised or running on a local oscillator within the required accuracy of UTC.

**Verification Method:** Unit Test
**Test Cases:**
- TC-019-550-001: Parse valid System Status
- TC-019-550-002: Verify format compliance
- TC-019-550-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat019_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-019-551: Parse Tracking Processor Detailed Status (I019/551)

**Parent:** REQ-HLR-CAT-019
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I019/551 (Tracking Processor Detailed Status) as specified in the EUROCONTROL ASTERIX CAT019 v1.3 specification.

**Data Item Definition:**
Information concerning the configuration and status of the Tracking processors.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: Both Bits of one TP set to zero means, that this TP is not used in the system. 

**Verification Method:** Unit Test
**Test Cases:**
- TC-019-551-001: Parse valid Tracking Processor Detailed Status
- TC-019-551-002: Verify format compliance
- TC-019-551-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat019_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-019-552: Parse Remote Sensor Detailed Status (I019/552)

**Parent:** REQ-HLR-CAT-019
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I019/552 (Remote Sensor Detailed Status) as specified in the EUROCONTROL ASTERIX CAT019 v1.3 specification.

**Data Item Definition:**
Information concerning the configuration and status of the Remote Sensors (RS)

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-019-552-001: Parse valid Remote Sensor Detailed Status
- TC-019-552-002: Verify format compliance
- TC-019-552-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat019_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-019-553: Parse Reference Transponder Detailed Status (I019/553)

**Parent:** REQ-HLR-CAT-019
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I019/553 (Reference Transponder Detailed Status) as specified in the EUROCONTROL ASTERIX CAT019 v1.3 specification.

**Data Item Definition:**
Information concerning the configuration and status of the Reference Transponder.

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: This goes on as many as Reference Transponders are available. Due to limitation in decoder we limit the number of transpoders to 50.

**Verification Method:** Unit Test
**Test Cases:**
- TC-019-553-001: Parse valid Reference Transponder Detailed Status
- TC-019-553-002: Verify format compliance
- TC-019-553-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat019_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-019-600: Parse Position of the MLT System Reference Point (I019/600)

**Parent:** REQ-HLR-CAT-019
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I019/600 (Position of the MLT System Reference Point) as specified in the EUROCONTROL ASTERIX CAT019 v1.3 specification.

**Data Item Definition:**
Position of the MLT reference point in WGS-84 Coordinates.

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-019-600-001: Parse valid Position of the MLT System Reference Point
- TC-019-600-002: Verify format compliance
- TC-019-600-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat019_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-019-610: Parse Height of the MLT System Reference Point (I019/610)

**Parent:** REQ-HLR-CAT-019
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I019/610 (Height of the MLT System Reference Point) as specified in the EUROCONTROL ASTERIX CAT019 v1.3 specification.

**Data Item Definition:**
Height of the MLT system reference point in two's complement form. The height shall use mean sea level as the zero reference level.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-019-610-001: Parse valid Height of the MLT System Reference Point
- TC-019-610-002: Verify format compliance
- TC-019-610-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat019_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-019-620: Parse WGS-84 Undulation (I019/620)

**Parent:** REQ-HLR-CAT-019
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I019/620 (WGS-84 Undulation) as specified in the EUROCONTROL ASTERIX CAT019 v1.3 specification.

**Data Item Definition:**
WGS-84 undulation value of the MLT system reference point, in meters. Geoid undulation value is the difference between the ellipsoidal height and the height above mean sea level

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-019-620-001: Parse valid WGS-84 Undulation
- TC-019-620-002: Verify format compliance
- TC-019-620-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat019_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-019-SP: Parse Special Purpose (I019/SP)

**Parent:** REQ-HLR-CAT-019
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I019/SP (Special Purpose) as specified in the EUROCONTROL ASTERIX CAT019 v1.3 specification.

**Data Item Definition:**
Special information

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-019-SP-001: Parse valid Special Purpose
- TC-019-SP-002: Verify format compliance
- TC-019-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat019_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-019-RE: Parse Reserved Expansion Field (I019/RE)

**Parent:** REQ-HLR-CAT-019
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I019/RE (Reserved Expansion Field) as specified in the EUROCONTROL ASTERIX CAT019 v1.3 specification.

**Data Item Definition:**
RE

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-019-RE-001: Parse valid Reserved Expansion Field
- TC-019-RE-002: Verify format compliance
- TC-019-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat019_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-019-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT019 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-019-ERR-001: Truncated data items
- TC-019-ERR-002: Invalid repetition counts
- TC-019-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-019-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT019 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-019-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-019-000 | I019/000 | Message Type | Critical | High |
| REQ-LLR-019-010 | I019/010 | Data Source Identifier | Critical | High |
| REQ-LLR-019-140 | I019/140 | Time of Day | Critical | High |
| REQ-LLR-019-550 | I019/550 | System Status | Medium | Low |
| REQ-LLR-019-551 | I019/551 | Tracking Processor Detailed Status | Medium | Low |
| REQ-LLR-019-552 | I019/552 | Remote Sensor Detailed Status | Medium | Low |
| REQ-LLR-019-553 | I019/553 | Reference Transponder Detailed Status | Medium | Low |
| REQ-LLR-019-600 | I019/600 | Position of the MLT System Reference Poi | Medium | Low |
| REQ-LLR-019-610 | I019/610 | Height of the MLT System Reference Point | Medium | Low |
| REQ-LLR-019-620 | I019/620 | WGS-84 Undulation | Medium | Low |
| REQ-LLR-019-SP | I019/SP | Special Purpose | Medium | Low |
| REQ-LLR-019-RE | I019/RE | Reserved Expansion Field | Medium | Low |
| REQ-LLR-019-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-019-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 14

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-019
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-019-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT019 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
