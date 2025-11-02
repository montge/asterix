# Low-Level Requirements - ASTERIX Category 34
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT34-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-34
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 34 (Transmission of Monoroadar Service Messages) v1.0.

**Parent Requirement:** REQ-HLR-CAT-34 - Parse ASTERIX Category 34

Each data item in CAT34 has corresponding LLRs defining parsing behavior.

---

## 2. CAT34 Data Item Requirements

### REQ-LLR-34-000: Parse Message Type (I34/000)

**Parent:** REQ-HLR-CAT-34
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I34/000 (Message Type) as specified in the EUROCONTROL ASTERIX CAT34 v1.0 specification.

**Data Item Definition:**
This Data Item allows for a more convenient handling of the messages at the receiver side by further defining the type of transaction.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-000-001: Parse valid Message Type
- TC-34-000-002: Verify format compliance
- TC-34-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-34-010: Parse Data Source Identifier (I34/010)

**Parent:** REQ-HLR-CAT-34
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I34/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT34 v1.0 specification.

**Data Item Definition:**
Identification of the radar station from which the data are received.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-010-001: Parse valid Data Source Identifier
- TC-34-010-002: Verify format compliance
- TC-34-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-34-020: Parse Sector Number (I34/020)

**Parent:** REQ-HLR-CAT-34
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I34/020 (Sector Number) as specified in the EUROCONTROL ASTERIX CAT34 v1.0 specification.

**Data Item Definition:**
Eight most significant bits of the antenna azimuth defining a particular azimuth sector.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: .

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-020-001: Parse valid Sector Number
- TC-34-020-002: Verify format compliance
- TC-34-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-34-030: Parse Time of Day (I34/030)

**Parent:** REQ-HLR-CAT-34
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I34/030 (Time of Day) as specified in the EUROCONTROL ASTERIX CAT34 v1.0 specification.

**Data Item Definition:**
Absolute time stamping expressed as UTC time.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: The time information, coded in three octets, shall reflect the exact time of an event (crossing of the azimuth defining the beginning of a sector by the antenna), expressed as a number of 1/128 s elapsed since last midnight. 

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-030-001: Parse valid Time of Day
- TC-34-030-002: Verify format compliance
- TC-34-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-34-041: Parse Antenna Rotation Speed (I34/041)

**Parent:** REQ-HLR-CAT-34
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I34/041 (Antenna Rotation Speed) as specified in the EUROCONTROL ASTERIX CAT34 v1.0 specification.

**Data Item Definition:**
Antenna rotation period as measured between two consecutive North crossings or as averaged during a period of time.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-041-001: Parse valid Antenna Rotation Speed
- TC-34-041-002: Verify format compliance
- TC-34-041-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-34-050: Parse System Configuration and Status (I34/050)

**Parent:** REQ-HLR-CAT-34
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I34/050 (System Configuration and Status) as specified in the EUROCONTROL ASTERIX CAT34 v1.0 specification.

**Data Item Definition:**
Information concerning the configuration and status of a System..

**Implementation Notes:**
- Format: Compound
- Rule: optional
- Note: Due to the diversity in hardware design and requirement of present and future radar stations, it is felt impractical to attempt to define the individual bits.

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-050-001: Parse valid System Configuration and Status
- TC-34-050-002: Verify format compliance
- TC-34-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-34-060: Parse System Processing Mode (I34/060)

**Parent:** REQ-HLR-CAT-34
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I34/060 (System Processing Mode) as specified in the EUROCONTROL ASTERIX CAT34 v1.0 specification.

**Data Item Definition:**
Details concerning the present status with respect to processing parameters and options.

**Implementation Notes:**
- Format: Compound
- Rule: optional
- Note: .

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-060-001: Parse valid System Processing Mode
- TC-34-060-002: Verify format compliance
- TC-34-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-34-070: Parse Message Count Values (I34/070)

**Parent:** REQ-HLR-CAT-34
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I34/070 (Message Count Values) as specified in the EUROCONTROL ASTERIX CAT34 v1.0 specification.

**Data Item Definition:**
Message Count values, according the various types of messages, for the last completed antenna revolution, counted between two North crossings

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-070-001: Parse valid Message Count Values
- TC-34-070-002: Verify format compliance
- TC-34-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-34-090: Parse Collimation Error (I34/090)

**Parent:** REQ-HLR-CAT-34
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I34/090 (Collimation Error) as specified in the EUROCONTROL ASTERIX CAT34 v1.0 specification.

**Data Item Definition:**
Averaged difference in range and in azimuth for the primary target position with respect to the SSR target position as calculated by the radar station.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-090-001: Parse valid Collimation Error
- TC-34-090-002: Verify format compliance
- TC-34-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-34-100: Parse Generic Polar Window (I34/100)

**Parent:** REQ-HLR-CAT-34
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I34/100 (Generic Polar Window) as specified in the EUROCONTROL ASTERIX CAT34 v1.0 specification.

**Data Item Definition:**
Geographical window defined in polar co-ordinates

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: optional
- Note: This data item shall be sent when: indicating a geographical filtering, indicating a jamming strobe area

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-100-001: Parse valid Generic Polar Window
- TC-34-100-002: Verify format compliance
- TC-34-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-34-110: Parse Data Filter (I34/110)

**Parent:** REQ-HLR-CAT-34
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I34/110 (Data Filter) as specified in the EUROCONTROL ASTERIX CAT34 v1.0 specification.

**Data Item Definition:**
.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: This Data Item is often used in conjunction with I034/100 and represents a  Data Filter for a specific geographical subarea. A Data Source may have  zero, one or multiple data filters active at any time.If I034/110 is not accompanied with I034/100, then the Data Filter is valid throughout the total area of coverage.

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-110-001: Parse valid Data Filter
- TC-34-110-002: Verify format compliance
- TC-34-110-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-34-120: Parse 3D POS (I34/120)

**Parent:** REQ-HLR-CAT-34
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I34/120 (3D POS) as specified in the EUROCONTROL ASTERIX CAT34 v1.0 specification.

**Data Item Definition:**
3D-Position of Data Source in WGS-84 Coordinates

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-120-001: Parse valid 3D POS
- TC-34-120-002: Verify format compliance
- TC-34-120-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-34-SP: Parse Special Purpose Field (I34/SP)

**Parent:** REQ-HLR-CAT-34
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I34/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT34 v1.0 specification.

**Data Item Definition:**
SP

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-SP-001: Parse valid Special Purpose Field
- TC-34-SP-002: Verify format compliance
- TC-34-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-34-RE: Parse Reserved Field (I34/RE)

**Parent:** REQ-HLR-CAT-34
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I34/RE (Reserved Field) as specified in the EUROCONTROL ASTERIX CAT34 v1.0 specification.

**Data Item Definition:**
RE

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-RE-001: Parse valid Reserved Field
- TC-34-RE-002: Verify format compliance
- TC-34-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-34-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT34 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-ERR-001: Truncated data items
- TC-34-ERR-002: Invalid repetition counts
- TC-34-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-34-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT34 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-34-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-34-000 | I34/000 | Message Type | Medium | Low |
| REQ-LLR-34-010 | I34/010 | Data Source Identifier | Medium | Low |
| REQ-LLR-34-020 | I34/020 | Sector Number | Medium | Low |
| REQ-LLR-34-030 | I34/030 | Time of Day | Medium | Low |
| REQ-LLR-34-041 | I34/041 | Antenna Rotation Speed | Medium | Low |
| REQ-LLR-34-050 | I34/050 | System Configuration and Status | Medium | Low |
| REQ-LLR-34-060 | I34/060 | System Processing Mode | Medium | Low |
| REQ-LLR-34-070 | I34/070 | Message Count Values | Medium | Low |
| REQ-LLR-34-090 | I34/090 | Collimation Error | Medium | Low |
| REQ-LLR-34-100 | I34/100 | Generic Polar Window | Medium | Low |
| REQ-LLR-34-110 | I34/110 | Data Filter | Medium | Low |
| REQ-LLR-34-120 | I34/120 | 3D POS | Medium | Low |
| REQ-LLR-34-SP | I34/SP | Special Purpose Field | Medium | Low |
| REQ-LLR-34-RE | I34/RE | Reserved Field | Medium | Low |
| REQ-LLR-34-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-34-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 16

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-34
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-34-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT34 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
