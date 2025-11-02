# Low-Level Requirements - ASTERIX Category 8
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT8-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-8
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 8 (Monoroadar Derived Weather Information) v1.0.

**Parent Requirement:** REQ-HLR-CAT-8 - Parse ASTERIX Category 8

Each data item in CAT8 has corresponding LLRs defining parsing behavior.

---

## 2. CAT8 Data Item Requirements

### REQ-LLR-8-000: Parse Message Type (I8/000)

**Parent:** REQ-HLR-CAT-8
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I8/000 (Message Type) as specified in the EUROCONTROL ASTERIX CAT8 v1.0 specification.

**Data Item Definition:**
This Data Item allows for a more convenient handling of the messages at the receiver side by further defining the type of transaction.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-8-000-001: Parse valid Message Type
- TC-8-000-002: Verify format compliance
- TC-8-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat008_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-8-010: Parse Data Source Identifier (I8/010)

**Parent:** REQ-HLR-CAT-8
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I8/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT8 v1.0 specification.

**Data Item Definition:**
Identification of the radar station from which the data are received.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-8-010-001: Parse valid Data Source Identifier
- TC-8-010-002: Verify format compliance
- TC-8-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat008_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-8-020: Parse Vector Qualifier (I8/020)

**Parent:** REQ-HLR-CAT-8
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I8/020 (Vector Qualifier) as specified in the EUROCONTROL ASTERIX CAT8 v1.0 specification.

**Data Item Definition:**
Precipitation intensity level, shading orientation of the vectors representing the precipitation area and coordinate system used.

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: For polar vectors bits-4/2 (1'st octet) are meaningless and are set to zero.

**Verification Method:** Unit Test
**Test Cases:**
- TC-8-020-001: Parse valid Vector Qualifier
- TC-8-020-002: Verify format compliance
- TC-8-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat008_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-8-034: Parse Sequence of Polar Vectors in SPF Notation (I8/034)

**Parent:** REQ-HLR-CAT-8
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I8/034 (Sequence of Polar Vectors in SPF Notation) as specified in the EUROCONTROL ASTERIX CAT8 v1.0 specification.

**Data Item Definition:**
Sequence of weather vectors in local polar coordinates.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: f is a parameter of the SOP message.

**Verification Method:** Unit Test
**Test Cases:**
- TC-8-034-001: Parse valid Sequence of Polar Vectors in SPF Notation
- TC-8-034-002: Verify format compliance
- TC-8-034-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat008_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-8-036: Parse Sequence of Cartesian Vectors in SPF Notation (I8/036)

**Parent:** REQ-HLR-CAT-8
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I8/036 (Sequence of Cartesian Vectors in SPF Notation) as specified in the EUROCONTROL ASTERIX CAT8 v1.0 specification.

**Data Item Definition:**
Sequence of weather vectors, in the representation start point/ length, in local or in system cartesian coordinates.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: f is a parameter of the SOP message. Negative values are expressed in 2's complement form, bit-24 and bit-16 are set to 0 for positive values and 1 for negative values.

**Verification Method:** Unit Test
**Test Cases:**
- TC-8-036-001: Parse valid Sequence of Cartesian Vectors in SPF Notation
- TC-8-036-002: Verify format compliance
- TC-8-036-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat008_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-8-038: Parse Sequence of Weather Vectors in SPF Notation (I8/038)

**Parent:** REQ-HLR-CAT-8
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I8/038 (Sequence of Weather Vectors in SPF Notation) as specified in the EUROCONTROL ASTERIX CAT8 v1.0 specification.

**Data Item Definition:**
Sequence of weather vectors, in the representation start point/ end point, in local or in system cartesian coordinates.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: f is a parameter of the SOP message. Negative values are expressed in 2's complement form, bits-32, 24, 16  and bit-8 are set to 0 for positive values and 1 for negative values.

**Verification Method:** Unit Test
**Test Cases:**
- TC-8-038-001: Parse valid Sequence of Weather Vectors in SPF Notation
- TC-8-038-002: Verify format compliance
- TC-8-038-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat008_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-8-040: Parse Contour Identifier (I8/040)

**Parent:** REQ-HLR-CAT-8
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I8/040 (Contour Identifier) as specified in the EUROCONTROL ASTERIX CAT8 v1.0 specification.

**Data Item Definition:**
Contour serial number together with the precipitation intensity levels and the coordinates system used.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The Contour Serial Number provides an unambiguous identification for each contour record. Within one update cycle, a serial number shall never be assigned twice.

**Verification Method:** Unit Test
**Test Cases:**
- TC-8-040-001: Parse valid Contour Identifier
- TC-8-040-002: Verify format compliance
- TC-8-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat008_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-8-050: Parse Sequence of Contour Points in SPF Notation (I8/050)

**Parent:** REQ-HLR-CAT-8
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I8/050 (Sequence of Contour Points in SPF Notation) as specified in the EUROCONTROL ASTERIX CAT8 v1.0 specification.

**Data Item Definition:**
Cartesian coordinates of a variable number of points defining a contour.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: f is a parameter of the SOP message. Negative values are expressed in 2's complement form, bit-16 and bit-8 are set to 0 for positive values and 1 for negative values.

**Verification Method:** Unit Test
**Test Cases:**
- TC-8-050-001: Parse valid Sequence of Contour Points in SPF Notation
- TC-8-050-002: Verify format compliance
- TC-8-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat008_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-8-090: Parse Time of Day (I8/090)

**Parent:** REQ-HLR-CAT-8
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I8/090 (Time of Day) as specified in the EUROCONTROL ASTERIX CAT8 v1.0 specification.

**Data Item Definition:**
Absolute time stamping expressed as UTC time.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: 1. The time of day value is reset to zero each day at midnight. 2. For time management in radar transmission applications, refer to Part 1, paragraph 5.4 [ Ref.2]. 

**Verification Method:** Unit Test
**Test Cases:**
- TC-8-090-001: Parse valid Time of Day
- TC-8-090-002: Verify format compliance
- TC-8-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat008_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-8-100: Parse Processing Status (I8/100)

**Parent:** REQ-HLR-CAT-8
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I8/100 (Processing Status) as specified in the EUROCONTROL ASTERIX CAT8 v1.0 specification.

**Data Item Definition:**
Information concerning the scaling factor currently applied, current reduction step in use, etc.

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-8-100-001: Parse valid Processing Status
- TC-8-100-002: Verify format compliance
- TC-8-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat008_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-8-110: Parse Station Configuration Status (I8/110)

**Parent:** REQ-HLR-CAT-8
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I8/110 (Station Configuration Status) as specified in the EUROCONTROL ASTERIX CAT8 v1.0 specification.

**Data Item Definition:**
Information concerning the use and status of some vital hardware components of the radar system.

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: Due to the diversity in hardware design and requirement of present and future radar stations, it is felt impractical to attempt to define the individual bits.

**Verification Method:** Unit Test
**Test Cases:**
- TC-8-110-001: Parse valid Station Configuration Status
- TC-8-110-002: Verify format compliance
- TC-8-110-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat008_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-8-120: Parse Total Number of Items Constituting One Weather (I8/120)

**Parent:** REQ-HLR-CAT-8
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I8/120 (Total Number of Items Constituting One Weather) as specified in the EUROCONTROL ASTERIX CAT8 v1.0 specification.

**Data Item Definition:**
Total number of vectors, respectively contour points, constituting the total weather image, provided with the EOP message.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-8-120-001: Parse valid Total Number of Items Constituting One Weather
- TC-8-120-002: Verify format compliance
- TC-8-120-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat008_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-8-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT8 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-8-ERR-001: Truncated data items
- TC-8-ERR-002: Invalid repetition counts
- TC-8-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-8-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT8 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-8-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-8-000 | I8/000 | Message Type | Medium | Low |
| REQ-LLR-8-010 | I8/010 | Data Source Identifier | Medium | Low |
| REQ-LLR-8-020 | I8/020 | Vector Qualifier | Medium | Low |
| REQ-LLR-8-034 | I8/034 | Sequence of Polar Vectors in SPF Notatio | Medium | Low |
| REQ-LLR-8-036 | I8/036 | Sequence of Cartesian Vectors in SPF Not | Medium | Low |
| REQ-LLR-8-038 | I8/038 | Sequence of Weather Vectors in SPF Notat | Medium | Low |
| REQ-LLR-8-040 | I8/040 | Contour Identifier | Medium | Low |
| REQ-LLR-8-050 | I8/050 | Sequence of Contour Points in SPF Notati | Medium | Low |
| REQ-LLR-8-090 | I8/090 | Time of Day | Medium | Low |
| REQ-LLR-8-100 | I8/100 | Processing Status | Medium | Low |
| REQ-LLR-8-110 | I8/110 | Station Configuration Status | Medium | Low |
| REQ-LLR-8-120 | I8/120 | Total Number of Items Constituting One W | Medium | Low |
| REQ-LLR-8-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-8-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 14

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-8
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-8-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT8 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
