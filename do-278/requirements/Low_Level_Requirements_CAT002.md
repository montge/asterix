# Low-Level Requirements - ASTERIX Category 002
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT002-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-002
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 002 (Transmission of Monoradar Service Messages) v1.1.

**Parent Requirement:** REQ-HLR-CAT-002 - Parse ASTERIX Category 002

Each data item in CAT002 has corresponding LLRs defining parsing behavior.

---

## 2. CAT002 Data Item Requirements

### REQ-LLR-002-000: Parse Message Type (I002/000)

**Parent:** REQ-HLR-CAT-002
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I002/000 (Message Type) as specified in the EUROCONTROL ASTERIX CAT002 v1.1 specification.

**Data Item Definition:**

            This Data Item allows for a more convenient handling of the
            messages at the receiver side by further defining the type of
            transaction.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
            Notes:
                1. In application where transactions of various types are exchanged, the
                Message Type Data Item facilitates the proper message handling at the
                receiver side.

                2. Message Type values 1-127 are reserved for common standard use,
                whereas the values 128-255 are application dependent.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-002-000-001: Parse valid Message Type
- TC-002-000-002: Verify format compliance
- TC-002-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat002_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-002-010: Parse Data Source Identifier (I002/010)

**Parent:** REQ-HLR-CAT-002
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I002/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT002 v1.1 specification.

**Data Item Definition:**

            Identification of the radar station from which the data are received.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Notes:
                1. The defined SACs are listed in Part 1, Table 2 [Ref. 2]
                2. The defined SICs are listed in Part 1, Annex B [Ref. 2]
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-002-010-001: Parse valid Data Source Identifier
- TC-002-010-002: Verify format compliance
- TC-002-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat002_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-002-020: Parse Sector Number (I002/020)

**Parent:** REQ-HLR-CAT-002
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I002/020 (Sector Number) as specified in the EUROCONTROL ASTERIX CAT002 v1.1 specification.

**Data Item Definition:**

            Eight most significant bits of the antenna azimuth defining a
            particular azimuth sector.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
            The use of the antenna azimuth as sector number has the
            advantage of being independent of the number of sectors
            implemented.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-002-020-001: Parse valid Sector Number
- TC-002-020-002: Verify format compliance
- TC-002-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat002_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-002-030: Parse Time of Day (I002/030)

**Parent:** REQ-HLR-CAT-002
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I002/030 (Time of Day) as specified in the EUROCONTROL ASTERIX CAT002 v1.1 specification.

**Data Item Definition:**

            Absolute time stamping expressed as UTC time.
        

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: 
            Notes:

                1. The time of day value is reset to zero each day at midnight.
                2. For time management in radar transmission applications, refer to Part 1,
                   paragraph 5.4 [ Ref.2].
                3. Data Item I002/030 can have various logical meanings. In a particular
                   message, the logical meaning is implicit from its context (e.g. in a North
                   marker message it represents the antenna North crossing time; in a
                   sector message it represents the antenna sector crossing time).
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-002-030-001: Parse valid Time of Day
- TC-002-030-002: Verify format compliance
- TC-002-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat002_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-002-041: Parse Antenna Rotation Speed (I002/041)

**Parent:** REQ-HLR-CAT-002
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I002/041 (Antenna Rotation Speed) as specified in the EUROCONTROL ASTERIX CAT002 v1.1 specification.

**Data Item Definition:**

            Antenna rotation period as measured between two
            consecutive North crossings or as averaged during a period
            of time.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-002-041-001: Parse valid Antenna Rotation Speed
- TC-002-041-002: Verify format compliance
- TC-002-041-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat002_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-002-050: Parse Station Configuration Status (I002/050)

**Parent:** REQ-HLR-CAT-002
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I002/050 (Station Configuration Status) as specified in the EUROCONTROL ASTERIX CAT002 v1.1 specification.

**Data Item Definition:**

            Information concerning the use and status of some vital
            hardware components of the radar system.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            Due to the diversity in hardware design and requirement of
            present and future radar stations, it is felt impractical to attempt to
            define the individual bits.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-002-050-001: Parse valid Station Configuration Status
- TC-002-050-002: Verify format compliance
- TC-002-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat002_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-002-060: Parse Station Processing Mode (I002/060)

**Parent:** REQ-HLR-CAT-002
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I002/060 (Station Processing Mode) as specified in the EUROCONTROL ASTERIX CAT002 v1.1 specification.

**Data Item Definition:**

            Details concerning the present status with respect to
            processing parameters and options.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            NOTES:
                1. Typical information conveyed within this Data Item includes inter alia
                   type of polarisation in use, Moving Target Indicator (MTI) in use and/or
                   definition of the range to which MTI is applied, presence of overload
                   conditions and the type of load reduction measures in use.
                2. Only the structure of this Data Item is defined, no attempt is made to
                   standardise its contents, in order not to hamper any application or future
                   development.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-002-060-001: Parse valid Station Processing Mode
- TC-002-060-002: Verify format compliance
- TC-002-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat002_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-002-070: Parse Plot Count Values (I002/070)

**Parent:** REQ-HLR-CAT-002
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I002/070 (Plot Count Values) as specified in the EUROCONTROL ASTERIX CAT002 v1.1 specification.

**Data Item Definition:**

            Plot count values according to various plot categories, either
            for the last full antenna scan or for the last sector processed.
        

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-002-070-001: Parse valid Plot Count Values
- TC-002-070-002: Verify format compliance
- TC-002-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat002_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-002-080: Parse Warning/Error Conditions (I002/080)

**Parent:** REQ-HLR-CAT-002
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I002/080 (Warning/Error Conditions) as specified in the EUROCONTROL ASTERIX CAT002 v1.1 specification.

**Data Item Definition:**

            Warning/error conditions affecting the functioning of the
            radar system itself.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            NOTE:
                Warning/error condition values 1-63 are reserved for common
                Standard use, whereas the values 64-127 are application
                dependent.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-002-080-001: Parse valid Warning/Error Conditions
- TC-002-080-002: Verify format compliance
- TC-002-080-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat002_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-002-090: Parse Collimation Error (I002/090)

**Parent:** REQ-HLR-CAT-002
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I002/090 (Collimation Error) as specified in the EUROCONTROL ASTERIX CAT002 v1.1 specification.

**Data Item Definition:**

            Averaged difference in range and in azimuth for the primary
            target position with respect to the SSR target position as
            calculated by the radar station.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            NOTES
                1. LSB of RE is calculated as :math:`2^{16-f}`.
                2. A default quantisation unit of 0.022deg and a range between -2.8125deg and
                   +2.7905deg is obtained for a value of f=2 .
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-002-090-001: Parse valid Collimation Error
- TC-002-090-002: Verify format compliance
- TC-002-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat002_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-002-100: Parse Dynamic Window Type 1 (I002/100)

**Parent:** REQ-HLR-CAT-002
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I002/100 (Dynamic Window Type 1) as specified in the EUROCONTROL ASTERIX CAT002 v1.1 specification.

**Data Item Definition:**

            Signals the activation of a certain selective filtering function
            and in a polar coordinates system the respective
            geographical areas.
        

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: optional
- Note: 
            The logical meaning of the polar window is defined by its context,
            given by the Message Type (Data Item I002/000) in the record
            concerned.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-002-100-001: Parse valid Dynamic Window Type 1
- TC-002-100-002: Verify format compliance
- TC-002-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat002_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-002-SP: Parse Special Purpose Field (I002/SP)

**Parent:** REQ-HLR-CAT-002
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I002/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT002 v1.1 specification.

**Data Item Definition:**

            Special Purpose Field
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-002-SP-001: Parse valid Special Purpose Field
- TC-002-SP-002: Verify format compliance
- TC-002-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat002_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-002-RFS: Parse Random Field Sequencing (I002/RFS)

**Parent:** REQ-HLR-CAT-002
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I002/RFS (Random Field Sequencing) as specified in the EUROCONTROL ASTERIX CAT002 v1.1 specification.

**Data Item Definition:**

            Random Field Sequencing
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-002-RFS-001: Parse valid Random Field Sequencing
- TC-002-RFS-002: Verify format compliance
- TC-002-RFS-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat002_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-002-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT002 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-002-ERR-001: Truncated data items
- TC-002-ERR-002: Invalid repetition counts
- TC-002-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-002-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT002 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-002-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-002-000 | I002/000 | Message Type | Medium | Low |
| REQ-LLR-002-010 | I002/010 | Data Source Identifier | Medium | Low |
| REQ-LLR-002-020 | I002/020 | Sector Number | Medium | Low |
| REQ-LLR-002-030 | I002/030 | Time of Day | Medium | Low |
| REQ-LLR-002-041 | I002/041 | Antenna Rotation Speed | Medium | Low |
| REQ-LLR-002-050 | I002/050 | Station Configuration Status | Medium | Low |
| REQ-LLR-002-060 | I002/060 | Station Processing Mode | Medium | Low |
| REQ-LLR-002-070 | I002/070 | Plot Count Values | Medium | Low |
| REQ-LLR-002-080 | I002/080 | Warning/Error Conditions | Medium | Low |
| REQ-LLR-002-090 | I002/090 | Collimation Error | Medium | Low |
| REQ-LLR-002-100 | I002/100 | Dynamic Window Type 1 | Medium | Low |
| REQ-LLR-002-SP | I002/SP | Special Purpose Field | Medium | Low |
| REQ-LLR-002-RFS | I002/RFS | Random Field Sequencing | Medium | Low |
| REQ-LLR-002-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-002-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 15

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-002
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-002-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT002 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
