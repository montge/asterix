# Low-Level Requirements - ASTERIX Category 025
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT025-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-025
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 025 (CNS/ATM Ground System Status Reports) v1.5.

**Parent Requirement:** REQ-HLR-CAT-025 - Parse ASTERIX Category 025

Each data item in CAT025 has corresponding LLRs defining parsing behavior.

---

## 2. CAT025 Data Item Requirements

### REQ-LLR-025-000: Parse Report Type (I025/000)

**Parent:** REQ-HLR-CAT-025
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I025/000 (Report Type) as specified in the EUROCONTROL ASTERIX CAT025 v1.5 specification.

**Data Item Definition:**
This Data Item allows for a more convenient handling of the reports at the receiver side by further defining the type of transaction.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-025-000-001: Parse valid Report Type
- TC-025-000-002: Verify format compliance
- TC-025-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat025_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-025-010: Parse Data Source Identifier (I025/010)

**Parent:** REQ-HLR-CAT-025
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I025/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT025 v1.5 specification.

**Data Item Definition:**
Identification of the Ground System sending the data.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-025-010-001: Parse valid Data Source Identifier
- TC-025-010-002: Verify format compliance
- TC-025-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat025_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-025-015: Parse Service Identification (I025/015)

**Parent:** REQ-HLR-CAT-025
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I025/015 (Service Identification) as specified in the EUROCONTROL ASTERIX CAT025 v1.5 specification.

**Data Item Definition:**
Identifies the service being reported.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: The service identification is allocated by the system

**Verification Method:** Unit Test
**Test Cases:**
- TC-025-015-001: Parse valid Service Identification
- TC-025-015-002: Verify format compliance
- TC-025-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat025_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-025-020: Parse Service Designator (I025/020)

**Parent:** REQ-HLR-CAT-025
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I025/020 (Service Designator) as specified in the EUROCONTROL ASTERIX CAT025 v1.5 specification.

**Data Item Definition:**
Designator of the service being reported.

**Implementation Notes:**
- Format: Fixed (6 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-025-020-001: Parse valid Service Designator
- TC-025-020-002: Verify format compliance
- TC-025-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat025_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-025-070: Parse Time Of Day (I025/070)

**Parent:** REQ-HLR-CAT-025
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I025/070 (Time Of Day) as specified in the EUROCONTROL ASTERIX CAT025 v1.5 specification.

**Data Item Definition:**
Absolute time stamping of the message (UTC) in the form of elapsed time since last midnight.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: The time of the day value is reset to zero each day at midnight.

**Verification Method:** Unit Test
**Test Cases:**
- TC-025-070-001: Parse valid Time Of Day
- TC-025-070-002: Verify format compliance
- TC-025-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat025_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-025-100: Parse System and Service Status (I025/100)

**Parent:** REQ-HLR-CAT-025
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I025/100 (System and Service Status) as specified in the EUROCONTROL ASTERIX CAT025 v1.5 specification.

**Data Item Definition:**
Information concerning the status of the Service Volume.

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-025-100-001: Parse valid System and Service Status
- TC-025-100-002: Verify format compliance
- TC-025-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat025_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-025-105: Parse System and Service Eror Codes (I025/105)

**Parent:** REQ-HLR-CAT-025
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I025/105 (System and Service Eror Codes) as specified in the EUROCONTROL ASTERIX CAT025 v1.5 specification.

**Data Item Definition:**
Erros Status of the System and the Service

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-025-105-001: Parse valid System and Service Eror Codes
- TC-025-105-002: Verify format compliance
- TC-025-105-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat025_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-025-120: Parse Component Status (I025/120)

**Parent:** REQ-HLR-CAT-025
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I025/120 (Component Status) as specified in the EUROCONTROL ASTERIX CAT025 v1.5 specification.

**Data Item Definition:**
Status of various system components and, when applicable, error codes.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-025-120-001: Parse valid Component Status
- TC-025-120-002: Verify format compliance
- TC-025-120-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat025_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-025-140: Parse Service Statistics (I025/140)

**Parent:** REQ-HLR-CAT-025
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I025/140 (Service Statistics) as specified in the EUROCONTROL ASTERIX CAT025 v1.5 specification.

**Data Item Definition:**
Statistics concerning the service.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-025-140-001: Parse valid Service Statistics
- TC-025-140-002: Verify format compliance
- TC-025-140-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat025_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-025-200: Parse Message Identification (I025/200)

**Parent:** REQ-HLR-CAT-025
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I025/200 (Message Identification) as specified in the EUROCONTROL ASTERIX CAT025 v1.5 specification.

**Data Item Definition:**
Identification of a unique message.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-025-200-001: Parse valid Message Identification
- TC-025-200-002: Verify format compliance
- TC-025-200-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat025_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-025-600: Parse Position of the System Reference Point (I025/600)

**Parent:** REQ-HLR-CAT-025
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I025/600 (Position of the System Reference Point) as specified in the EUROCONTROL ASTERIX CAT025 v1.5 specification.

**Data Item Definition:**
Position of the reference point in WGS-84 Coordinates

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-025-600-001: Parse valid Position of the System Reference Point
- TC-025-600-002: Verify format compliance
- TC-025-600-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat025_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-025-610: Parse Height of the System Reference Point (I025/610)

**Parent:** REQ-HLR-CAT-025
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I025/610 (Height of the System Reference Point) as specified in the EUROCONTROL ASTERIX CAT025 v1.5 specification.

**Data Item Definition:**
Height of the reference point relative to Mean Sea Level

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-025-610-001: Parse valid Height of the System Reference Point
- TC-025-610-002: Verify format compliance
- TC-025-610-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat025_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-025-SP: Parse Special Purpose Field (I025/SP)

**Parent:** REQ-HLR-CAT-025
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I025/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT025 v1.5 specification.

**Data Item Definition:**
SP

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-025-SP-001: Parse valid Special Purpose Field
- TC-025-SP-002: Verify format compliance
- TC-025-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat025_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-025-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT025 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-025-ERR-001: Truncated data items
- TC-025-ERR-002: Invalid repetition counts
- TC-025-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-025-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT025 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-025-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-025-000 | I025/000 | Report Type | Medium | Low |
| REQ-LLR-025-010 | I025/010 | Data Source Identifier | Medium | Low |
| REQ-LLR-025-015 | I025/015 | Service Identification | Medium | Low |
| REQ-LLR-025-020 | I025/020 | Service Designator | Medium | Low |
| REQ-LLR-025-070 | I025/070 | Time Of Day | Medium | Low |
| REQ-LLR-025-100 | I025/100 | System and Service Status | Medium | Low |
| REQ-LLR-025-105 | I025/105 | System and Service Eror Codes | Medium | Low |
| REQ-LLR-025-120 | I025/120 | Component Status | Medium | Low |
| REQ-LLR-025-140 | I025/140 | Service Statistics | Medium | Low |
| REQ-LLR-025-200 | I025/200 | Message Identification | Medium | Low |
| REQ-LLR-025-600 | I025/600 | Position of the System Reference Point | Medium | Low |
| REQ-LLR-025-610 | I025/610 | Height of the System Reference Point | Medium | Low |
| REQ-LLR-025-SP | I025/SP | Special Purpose Field | Medium | Low |
| REQ-LLR-025-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-025-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 15

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-025
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-025-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT025 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
