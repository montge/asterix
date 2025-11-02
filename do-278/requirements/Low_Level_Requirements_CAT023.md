# Low-Level Requirements - ASTERIX Category 023
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT023-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-023
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 023 (CNS/ATM Ground Station and Service Status Reports) v1.3.

**Parent Requirement:** REQ-HLR-CAT-023 - Parse ASTERIX Category 023

Each data item in CAT023 has corresponding LLRs defining parsing behavior.

---

## 2. CAT023 Data Item Requirements

### REQ-LLR-023-000: Parse Report Type (I023/000)

**Parent:** REQ-HLR-CAT-023
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I023/000 (Report Type) as specified in the EUROCONTROL ASTERIX CAT023 v1.3 specification.

**Data Item Definition:**
This Data Item allows for a more convenient handling of the reports at the receiver side by further defining the type of transaction.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-023-000-001: Parse valid Report Type
- TC-023-000-002: Verify format compliance
- TC-023-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat023_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-023-010: Parse Data Source Identifier (I023/010)

**Parent:** REQ-HLR-CAT-023
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I023/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT023 v1.3 specification.

**Data Item Definition:**
Identification of the Ground Station from which the data is received.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-023-010-001: Parse valid Data Source Identifier
- TC-023-010-002: Verify format compliance
- TC-023-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat023_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-023-015: Parse Service Type and Identification (I023/015)

**Parent:** REQ-HLR-CAT-023
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I023/015 (Service Type and Identification) as specified in the EUROCONTROL ASTERIX CAT023 v1.3 specification.

**Data Item Definition:**
Identifies the type of service being reported.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-023-015-001: Parse valid Service Type and Identification
- TC-023-015-002: Verify format compliance
- TC-023-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat023_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-023-070: Parse Time of Day (I023/070)

**Parent:** REQ-HLR-CAT-023
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I023/070 (Time of Day) as specified in the EUROCONTROL ASTERIX CAT023 v1.3 specification.

**Data Item Definition:**
Absolute time stamping expressed as UTC time.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: mandatory
- Note: The time of day value is reset to zero each day at midnight

**Verification Method:** Unit Test
**Test Cases:**
- TC-023-070-001: Parse valid Time of Day
- TC-023-070-002: Verify format compliance
- TC-023-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat023_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-023-100: Parse Ground Station Status (I023/100)

**Parent:** REQ-HLR-CAT-023
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I023/100 (Ground Station Status) as specified in the EUROCONTROL ASTERIX CAT023 v1.3 specification.

**Data Item Definition:**
Information concerning the status of a Ground Station.

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-023-100-001: Parse valid Ground Station Status
- TC-023-100-002: Verify format compliance
- TC-023-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat023_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-023-101: Parse Service Configuration (I023/101)

**Parent:** REQ-HLR-CAT-023
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I023/101 (Service Configuration) as specified in the EUROCONTROL ASTERIX CAT023 v1.3 specification.

**Data Item Definition:**
Information concerning the configuration of a Service.

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-023-101-001: Parse valid Service Configuration
- TC-023-101-002: Verify format compliance
- TC-023-101-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat023_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-023-110: Parse Service Status (I023/110)

**Parent:** REQ-HLR-CAT-023
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I023/110 (Service Status) as specified in the EUROCONTROL ASTERIX CAT023 v1.3 specification.

**Data Item Definition:**
Information concerning the status of the Service provided by a Ground Station.

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-023-110-001: Parse valid Service Status
- TC-023-110-002: Verify format compliance
- TC-023-110-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat023_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-023-120: Parse Service Statistics (I023/120)

**Parent:** REQ-HLR-CAT-023
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I023/120 (Service Statistics) as specified in the EUROCONTROL ASTERIX CAT023 v1.3 specification.

**Data Item Definition:**
Statistics concerning the service. Provides counts of various ADS-B message types that have been received since the report was last sent.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-023-120-001: Parse valid Service Statistics
- TC-023-120-002: Verify format compliance
- TC-023-120-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat023_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-023-200: Parse Operational Range (I023/200)

**Parent:** REQ-HLR-CAT-023
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I023/200 (Operational Range) as specified in the EUROCONTROL ASTERIX CAT023 v1.3 specification.

**Data Item Definition:**
Currently active operational range of the Ground Station.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-023-200-001: Parse valid Operational Range
- TC-023-200-002: Verify format compliance
- TC-023-200-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat023_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-023-SP: Parse Special Purpose Field (I023/SP)

**Parent:** REQ-HLR-CAT-023
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I023/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT023 v1.3 specification.

**Data Item Definition:**
SP

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-023-SP-001: Parse valid Special Purpose Field
- TC-023-SP-002: Verify format compliance
- TC-023-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat023_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-023-RE: Parse Reserved Field (I023/RE)

**Parent:** REQ-HLR-CAT-023
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I023/RE (Reserved Field) as specified in the EUROCONTROL ASTERIX CAT023 v1.3 specification.

**Data Item Definition:**
RE

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-023-RE-001: Parse valid Reserved Field
- TC-023-RE-002: Verify format compliance
- TC-023-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat023_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-023-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT023 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-023-ERR-001: Truncated data items
- TC-023-ERR-002: Invalid repetition counts
- TC-023-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-023-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT023 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-023-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-023-000 | I023/000 | Report Type | Critical | High |
| REQ-LLR-023-010 | I023/010 | Data Source Identifier | Critical | High |
| REQ-LLR-023-015 | I023/015 | Service Type and Identification | Medium | Low |
| REQ-LLR-023-070 | I023/070 | Time of Day | Critical | High |
| REQ-LLR-023-100 | I023/100 | Ground Station Status | Medium | Low |
| REQ-LLR-023-101 | I023/101 | Service Configuration | Medium | Low |
| REQ-LLR-023-110 | I023/110 | Service Status | Medium | Low |
| REQ-LLR-023-120 | I023/120 | Service Statistics | Medium | Low |
| REQ-LLR-023-200 | I023/200 | Operational Range | Medium | Low |
| REQ-LLR-023-SP | I023/SP | Special Purpose Field | Medium | Low |
| REQ-LLR-023-RE | I023/RE | Reserved Field | Medium | Low |
| REQ-LLR-023-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-023-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 13

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-023
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-023-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT023 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
