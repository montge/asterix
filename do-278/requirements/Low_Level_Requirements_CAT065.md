# Low-Level Requirements - ASTERIX Category 65
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT65-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-65
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 65 (SDPS Service Status Messages) v1.3.

**Parent Requirement:** REQ-HLR-CAT-65 - Parse ASTERIX Category 65

Each data item in CAT65 has corresponding LLRs defining parsing behavior.

---

## 2. CAT65 Data Item Requirements

### REQ-LLR-65-000: Parse Message Type (I65/000)

**Parent:** REQ-HLR-CAT-65
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I65/000 (Message Type) as specified in the EUROCONTROL ASTERIX CAT65 v1.3 specification.

**Data Item Definition:**
This Data Item allows for a more convenient handling of the messages at the receiver side by further defining the type of transaction.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-65-000-001: Parse valid Message Type
- TC-65-000-002: Verify format compliance
- TC-65-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-65-010: Parse Data Source Identifier (I65/010)

**Parent:** REQ-HLR-CAT-65
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I65/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT65 v1.3 specification.

**Data Item Definition:**
Identification of the SDPS sending the data.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-65-010-001: Parse valid Data Source Identifier
- TC-65-010-002: Verify format compliance
- TC-65-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-65-015: Parse Service Identification (I65/015)

**Parent:** REQ-HLR-CAT-65
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I65/015 (Service Identification) as specified in the EUROCONTROL ASTERIX CAT65 v1.3 specification.

**Data Item Definition:**
Identification of the service provided to one or more users.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: the service identification is allocated by the SDPS

**Verification Method:** Unit Test
**Test Cases:**
- TC-65-015-001: Parse valid Service Identification
- TC-65-015-002: Verify format compliance
- TC-65-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-65-020: Parse Batch Number (I65/020)

**Parent:** REQ-HLR-CAT-65
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I65/020 (Batch Number) as specified in the EUROCONTROL ASTERIX CAT65 v1.3 specification.

**Data Item Definition:**
A number indicating the completion of a service for that batch of track data, from 0 to N-1, N being the number of batches used to make one complete processing cycle.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-65-020-001: Parse valid Batch Number
- TC-65-020-002: Verify format compliance
- TC-65-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-65-030: Parse Time Of Message (I65/030)

**Parent:** REQ-HLR-CAT-65
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I65/030 (Time Of Message) as specified in the EUROCONTROL ASTERIX CAT65 v1.3 specification.

**Data Item Definition:**
Absolute time stamping of the message in the form of elapsed time since last midnight.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: The time of the day value is reset to zero at every midnight.

**Verification Method:** Unit Test
**Test Cases:**
- TC-65-030-001: Parse valid Time Of Message
- TC-65-030-002: Verify format compliance
- TC-65-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-65-040: Parse SDPS Configuration and Status (I65/040)

**Parent:** REQ-HLR-CAT-65
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I65/040 (SDPS Configuration and Status) as specified in the EUROCONTROL ASTERIX CAT65 v1.3 specification.

**Data Item Definition:**
Status of an SDPS.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-65-040-001: Parse valid SDPS Configuration and Status
- TC-65-040-002: Verify format compliance
- TC-65-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-65-050: Parse Service Status Report (I65/050)

**Parent:** REQ-HLR-CAT-65
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I65/050 (Service Status Report) as specified in the EUROCONTROL ASTERIX CAT65 v1.3 specification.

**Data Item Definition:**
Report sent by the SDPS related to a service

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-65-050-001: Parse valid Service Status Report
- TC-65-050-002: Verify format compliance
- TC-65-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-65-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT65 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-65-ERR-001: Truncated data items
- TC-65-ERR-002: Invalid repetition counts
- TC-65-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-65-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT65 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-65-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-65-000 | I65/000 | Message Type | Medium | Low |
| REQ-LLR-65-010 | I65/010 | Data Source Identifier | Medium | Low |
| REQ-LLR-65-015 | I65/015 | Service Identification | Medium | Low |
| REQ-LLR-65-020 | I65/020 | Batch Number | Medium | Low |
| REQ-LLR-65-030 | I65/030 | Time Of Message | Medium | Low |
| REQ-LLR-65-040 | I65/040 | SDPS Configuration and Status | Medium | Low |
| REQ-LLR-65-050 | I65/050 | Service Status Report | Medium | Low |
| REQ-LLR-65-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-65-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 9

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-65
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-65-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT65 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
