# Low-Level Requirements - ASTERIX Category 034
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT034-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-034
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 034 (Transmission of Monoradar Service Messages) v1.29.

**Parent Requirement:** REQ-HLR-CAT-034 - Parse ASTERIX Category 034

Each data item in CAT034 has corresponding LLRs defining parsing behavior.

---

## 2. CAT034 Data Item Requirements

### REQ-LLR-034-000: Parse Message Type (I034/000)

**Parent:** REQ-HLR-CAT-034
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I034/000 (Message Type) as specified in the EUROCONTROL ASTERIX CAT034 v1.29 specification.

**Data Item Definition:**

            This Data Item allows for a more convenient handling of
            the messages at the receiver side by further defining
            the type of transaction.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
            Notes:

                1. In applications where transactions of various
                   types are exchanged, the Message Type Data Item facilitates the
                   proper message handling at the receiver side.
                2. All Message Type values are reserved for common standard use.
                3. The list of items present for the four message types is defined in
                   the following table.

                   M stands for mandatory, O for optional, X for never present.

                   TODO: message types table
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-000-001: Parse valid Message Type
- TC-034-000-002: Verify format compliance
- TC-034-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-034-010: Parse Data Source Identifier (I034/010)

**Parent:** REQ-HLR-CAT-034
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I034/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT034 v1.29 specification.

**Data Item Definition:**

            Identification of the radar station from which the data are received.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Note:
                - The up-to-date list of SACs is published on the
                  EUROCONTROL Web Site (http://www.eurocontrol.int/asterix).
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-010-001: Parse valid Data Source Identifier
- TC-034-010-002: Verify format compliance
- TC-034-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-034-020: Parse Sector Number (I034/020)

**Parent:** REQ-HLR-CAT-034
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I034/020 (Sector Number) as specified in the EUROCONTROL ASTERIX CAT034 v1.29 specification.

**Data Item Definition:**

            Eight most significant bits of the antenna azimuth defining a particular azimuth sector.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-020-001: Parse valid Sector Number
- TC-034-020-002: Verify format compliance
- TC-034-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-034-030: Parse Time of Day (I034/030)

**Parent:** REQ-HLR-CAT-034
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I034/030 (Time of Day) as specified in the EUROCONTROL ASTERIX CAT034 v1.29 specification.

**Data Item Definition:**

            Absolute time stamping expressed as UTC time.
        

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: 
            Notes:

                - The time of day value is reset to zero each day at midnight.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-030-001: Parse valid Time of Day
- TC-034-030-002: Verify format compliance
- TC-034-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-034-041: Parse Antenna Rotation Speed (I034/041)

**Parent:** REQ-HLR-CAT-034
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I034/041 (Antenna Rotation Speed) as specified in the EUROCONTROL ASTERIX CAT034 v1.29 specification.

**Data Item Definition:**

            Antenna rotation period as measured between two consecutive North crossings
            or as averaged during a period of time.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Notes:

                - This item represents the antenna rotation period as measured by the
                  radar station between two consecutive North crossings, or a calculated
                  antenna rotation speed as averaged during a period of time, or during a
                  number of antenna rotation scans.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-041-001: Parse valid Antenna Rotation Speed
- TC-034-041-002: Verify format compliance
- TC-034-041-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-034-050: Parse System Configuration and Status (I034/050)

**Parent:** REQ-HLR-CAT-034
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I034/050 (System Configuration and Status) as specified in the EUROCONTROL ASTERIX CAT034 v1.29 specification.

**Data Item Definition:**

            Information concerning the configuration and status of a System.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-050-001: Parse valid System Configuration and Status
- TC-034-050-002: Verify format compliance
- TC-034-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-034-060: Parse System Processing Mode (I034/060)

**Parent:** REQ-HLR-CAT-034
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I034/060 (System Processing Mode) as specified in the EUROCONTROL ASTERIX CAT034 v1.29 specification.

**Data Item Definition:**

            Status concerning the processing options, in use during the last antenna
            revolution, for the various Sensors, composing the System.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional
- Note: 
            Notes:

                - Applicable to all defined secondary subfields. The actual mapping
                  between the up to seven data reduction steps and their associated
                  data reduction measures is not subject to standardisation.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-060-001: Parse valid System Processing Mode
- TC-034-060-002: Verify format compliance
- TC-034-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-034-070: Parse Message Count Values (I034/070)

**Parent:** REQ-HLR-CAT-034
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I034/070 (Message Count Values) as specified in the EUROCONTROL ASTERIX CAT034 v1.29 specification.

**Data Item Definition:**

            Message Count values, according the various types of messages, for the
            last completed antenna revolution, counted between two North crossings
        

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-070-001: Parse valid Message Count Values
- TC-034-070-002: Verify format compliance
- TC-034-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-034-090: Parse Collimation Error (I034/090)

**Parent:** REQ-HLR-CAT-034
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I034/090 (Collimation Error) as specified in the EUROCONTROL ASTERIX CAT034 v1.29 specification.

**Data Item Definition:**

            Averaged difference in range and in azimuth for the primary target position
            with respect to the SSR target position as calculated by the radar station.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Notes:

                - Negative values are coded in two's complement form.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-090-001: Parse valid Collimation Error
- TC-034-090-002: Verify format compliance
- TC-034-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-034-100: Parse Generic Polar Window (I034/100)

**Parent:** REQ-HLR-CAT-034
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I034/100 (Generic Polar Window) as specified in the EUROCONTROL ASTERIX CAT034 v1.29 specification.

**Data Item Definition:**

            Geographical window defined in polar co-ordinates.
        

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-100-001: Parse valid Generic Polar Window
- TC-034-100-002: Verify format compliance
- TC-034-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-034-110: Parse Data Filter (I034/110)

**Parent:** REQ-HLR-CAT-034
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I034/110 (Data Filter) as specified in the EUROCONTROL ASTERIX CAT034 v1.29 specification.

**Data Item Definition:**

            Data Filter, which allows suppression of individual data types.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
            Notes:

                1. This Data Item is often used in conjunction with I034/100 and
                   represents a Data Filter for a specific geographical subarea.
                   A Data Source may have zero, one or multiple data filters active at any time.
                2. If I034/110 is not accompanied with I034/100, then the Data Filter
                   is valid throughout the total area of coverage.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-110-001: Parse valid Data Filter
- TC-034-110-002: Verify format compliance
- TC-034-110-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-034-120: Parse 3D-Position Of Data Source (I034/120)

**Parent:** REQ-HLR-CAT-034
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I034/120 (3D-Position Of Data Source) as specified in the EUROCONTROL ASTERIX CAT034 v1.29 specification.

**Data Item Definition:**

            3D-Position of Data Source in WGS 84 Co-ordinates
        

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-120-001: Parse valid 3D-Position Of Data Source
- TC-034-120-002: Verify format compliance
- TC-034-120-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-034-RE: Parse Reserved Expansion Field (I034/RE)

**Parent:** REQ-HLR-CAT-034
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I034/RE (Reserved Expansion Field) as specified in the EUROCONTROL ASTERIX CAT034 v1.29 specification.

**Data Item Definition:**

            Expansion
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-RE-001: Parse valid Reserved Expansion Field
- TC-034-RE-002: Verify format compliance
- TC-034-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-034-SP: Parse Special Purpose Field (I034/SP)

**Parent:** REQ-HLR-CAT-034
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I034/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT034 v1.29 specification.

**Data Item Definition:**

            Special Purpose Field
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-SP-001: Parse valid Special Purpose Field
- TC-034-SP-002: Verify format compliance
- TC-034-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat034_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-034-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT034 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-ERR-001: Truncated data items
- TC-034-ERR-002: Invalid repetition counts
- TC-034-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-034-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT034 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-034-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-034-000 | I034/000 | Message Type | Medium | Low |
| REQ-LLR-034-010 | I034/010 | Data Source Identifier | Medium | Low |
| REQ-LLR-034-020 | I034/020 | Sector Number | Medium | Low |
| REQ-LLR-034-030 | I034/030 | Time of Day | Medium | Low |
| REQ-LLR-034-041 | I034/041 | Antenna Rotation Speed | Medium | Low |
| REQ-LLR-034-050 | I034/050 | System Configuration and Status | Medium | Low |
| REQ-LLR-034-060 | I034/060 | System Processing Mode | Medium | Low |
| REQ-LLR-034-070 | I034/070 | Message Count Values | Medium | Low |
| REQ-LLR-034-090 | I034/090 | Collimation Error | Medium | Low |
| REQ-LLR-034-100 | I034/100 | Generic Polar Window | Medium | Low |
| REQ-LLR-034-110 | I034/110 | Data Filter | Medium | Low |
| REQ-LLR-034-120 | I034/120 | 3D-Position Of Data Source | Medium | Low |
| REQ-LLR-034-RE | I034/RE | Reserved Expansion Field | Medium | Low |
| REQ-LLR-034-SP | I034/SP | Special Purpose Field | Medium | Low |
| REQ-LLR-034-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-034-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 16

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-034
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-034-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT034 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
