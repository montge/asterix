# Low-Level Requirements - ASTERIX Category 065
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT065-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-065
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 065 (SDPS Service Status Reports) v1.5.

**Parent Requirement:** REQ-HLR-CAT-065 - Parse ASTERIX Category 065

Each data item in CAT065 has corresponding LLRs defining parsing behavior.

---

## 2. CAT065 Data Item Requirements

### REQ-LLR-065-000: Parse Message Type (I065/000)

**Parent:** REQ-HLR-CAT-065
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I065/000 (Message Type) as specified in the EUROCONTROL ASTERIX CAT065 v1.5 specification.

**Data Item Definition:**

            This Data Item allows for a more convenient handling of the
            messages at the receiver side by further defining the type of
            transaction.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
            Notes:
                - In application where transactions of various types are exchanged, the
                  Message Type Data Item facilitates the proper message handling at the
                  receiver side.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-065-000-001: Parse valid Message Type
- TC-065-000-002: Verify format compliance
- TC-065-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-065-010: Parse Data Source Identifier (I065/010)

**Parent:** REQ-HLR-CAT-065
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I065/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT065 v1.5 specification.

**Data Item Definition:**

            Identification of the system sending the data.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Note:
                - The up-to-date list of SACs is published on the
                  EUROCONTROL Web Site (http://www.eurocontrol.int/asterix).
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-065-010-001: Parse valid Data Source Identifier
- TC-065-010-002: Verify format compliance
- TC-065-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-065-015: Parse Service Identification (I065/015)

**Parent:** REQ-HLR-CAT-065
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I065/015 (Service Identification) as specified in the EUROCONTROL ASTERIX CAT065 v1.5 specification.

**Data Item Definition:**

            Identification of the service provided to one or more users.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
            Note:
                - The service identification is allocated by the SDPS.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-065-015-001: Parse valid Service Identification
- TC-065-015-002: Verify format compliance
- TC-065-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-065-020: Parse Batch Number (I065/020)

**Parent:** REQ-HLR-CAT-065
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I065/020 (Batch Number) as specified in the EUROCONTROL ASTERIX CAT065 v1.5 specification.

**Data Item Definition:**

            A number indicating the completion of a service for that batch of track
            data, from 0 to N-1, N being the number of batches used to make
            one complete processing cycle.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-065-020-001: Parse valid Batch Number
- TC-065-020-002: Verify format compliance
- TC-065-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-065-030: Parse Time of Message (I065/030)

**Parent:** REQ-HLR-CAT-065
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I065/030 (Time of Message) as specified in the EUROCONTROL ASTERIX CAT065 v1.5 specification.

**Data Item Definition:**

            Absolute time stamping of the message, in the form of elapsed time
            since last midnight, expressed as UTC.
        

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: 
            Note:
                - The time is reset to zero at every midnight.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-065-030-001: Parse valid Time of Message
- TC-065-030-002: Verify format compliance
- TC-065-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-065-040: Parse SDPS Configuration and Status (I065/040)

**Parent:** REQ-HLR-CAT-065
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I065/040 (SDPS Configuration and Status) as specified in the EUROCONTROL ASTERIX CAT065 v1.5 specification.

**Data Item Definition:**

            Status of an SDPS.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-065-040-001: Parse valid SDPS Configuration and Status
- TC-065-040-002: Verify format compliance
- TC-065-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-065-050: Parse Service Status Report (I065/050)

**Parent:** REQ-HLR-CAT-065
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I065/050 (Service Status Report) as specified in the EUROCONTROL ASTERIX CAT065 v1.5 specification.

**Data Item Definition:**

            Report sent by the SDPS related to a service
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-065-050-001: Parse valid Service Status Report
- TC-065-050-002: Verify format compliance
- TC-065-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-065-RE: Parse Reserved Expansion Field (I065/RE)

**Parent:** REQ-HLR-CAT-065
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I065/RE (Reserved Expansion Field) as specified in the EUROCONTROL ASTERIX CAT065 v1.5 specification.

**Data Item Definition:**

            Expansion
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-065-RE-001: Parse valid Reserved Expansion Field
- TC-065-RE-002: Verify format compliance
- TC-065-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-065-SP: Parse Special Purpose Field (I065/SP)

**Parent:** REQ-HLR-CAT-065
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I065/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT065 v1.5 specification.

**Data Item Definition:**

            Special Purpose Field
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-065-SP-001: Parse valid Special Purpose Field
- TC-065-SP-002: Verify format compliance
- TC-065-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat065_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-065-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT065 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-065-ERR-001: Truncated data items
- TC-065-ERR-002: Invalid repetition counts
- TC-065-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-065-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT065 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-065-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-065-000 | I065/000 | Message Type | Medium | Low |
| REQ-LLR-065-010 | I065/010 | Data Source Identifier | Medium | Low |
| REQ-LLR-065-015 | I065/015 | Service Identification | Medium | Low |
| REQ-LLR-065-020 | I065/020 | Batch Number | Medium | Low |
| REQ-LLR-065-030 | I065/030 | Time of Message | Medium | Low |
| REQ-LLR-065-040 | I065/040 | SDPS Configuration and Status | Medium | Low |
| REQ-LLR-065-050 | I065/050 | Service Status Report | Medium | Low |
| REQ-LLR-065-RE | I065/RE | Reserved Expansion Field | Medium | Low |
| REQ-LLR-065-SP | I065/SP | Special Purpose Field | Medium | Low |
| REQ-LLR-065-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-065-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 11

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-065
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-065-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT065 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
