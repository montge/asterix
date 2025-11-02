# Low-Level Requirements - ASTERIX Category 247
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT247-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-247
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 247 (Version Number Exchange) v1.3.

**Parent Requirement:** REQ-HLR-CAT-247 - Parse ASTERIX Category 247

Each data item in CAT247 has corresponding LLRs defining parsing behavior.

---

## 2. CAT247 Data Item Requirements

### REQ-LLR-247-010: Parse Data Source Identifier (I247/010)

**Parent:** REQ-HLR-CAT-247
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I247/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT247 v1.3 specification.

**Data Item Definition:**

            Identification of the radar station from which the data are received.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Note:
                The defined SACs are on the EUROCONTROL ASTERIX website
                (www.eurocontrol.int/asterix)
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-247-010-001: Parse valid Data Source Identifier
- TC-247-010-002: Verify format compliance
- TC-247-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat247_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-247-015: Parse Service Identification (I247/015)

**Parent:** REQ-HLR-CAT-247
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I247/015 (Service Identification) as specified in the EUROCONTROL ASTERIX CAT247 v1.3 specification.

**Data Item Definition:**

            Identification of the service provided to one or more users.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
            The service identification is allocated by the system.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-247-015-001: Parse valid Service Identification
- TC-247-015-002: Verify format compliance
- TC-247-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat247_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-247-140: Parse Time of Day (I247/140)

**Parent:** REQ-HLR-CAT-247
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I247/140 (Time of Day) as specified in the EUROCONTROL ASTERIX CAT247 v1.3 specification.

**Data Item Definition:**

            Absolute time stamping expressed as UTC.
        

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: 
            The time of day value is reset to zero each day at midnight.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-247-140-001: Parse valid Time of Day
- TC-247-140-002: Verify format compliance
- TC-247-140-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat247_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-247-550: Parse Category Version Number Report (I247/550)

**Parent:** REQ-HLR-CAT-247
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I247/550 (Category Version Number Report) as specified in the EUROCONTROL ASTERIX CAT247 v1.3 specification.

**Data Item Definition:**

            Version number of Categories used.
        

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: 
            Notes:

                1. The Version Number corresponds to the Edition of the ASTERIX
                   Category specification
                2. The version number(s) corresponding to the Edition of the ASTERIX
                   Category Reserved Expansion Field(s) can be specified in the REF.
                3. The version number(s) corresponding to the Edition of the ASTERIX
                   Category Special Purpose Field(s) may be specified in the SPF.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-247-550-001: Parse valid Category Version Number Report
- TC-247-550-002: Verify format compliance
- TC-247-550-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat247_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-247-RE: Parse Reserved Expansion Field (I247/RE)

**Parent:** REQ-HLR-CAT-247
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I247/RE (Reserved Expansion Field) as specified in the EUROCONTROL ASTERIX CAT247 v1.3 specification.

**Data Item Definition:**

            Expansion
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-247-RE-001: Parse valid Reserved Expansion Field
- TC-247-RE-002: Verify format compliance
- TC-247-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat247_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-247-SP: Parse Special Purpose Field (I247/SP)

**Parent:** REQ-HLR-CAT-247
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I247/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT247 v1.3 specification.

**Data Item Definition:**

            Special Purpose Field
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-247-SP-001: Parse valid Special Purpose Field
- TC-247-SP-002: Verify format compliance
- TC-247-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat247_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-247-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT247 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-247-ERR-001: Truncated data items
- TC-247-ERR-002: Invalid repetition counts
- TC-247-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-247-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT247 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-247-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-247-010 | I247/010 | Data Source Identifier | Medium | Low |
| REQ-LLR-247-015 | I247/015 | Service Identification | Medium | Low |
| REQ-LLR-247-140 | I247/140 | Time of Day | Medium | Low |
| REQ-LLR-247-550 | I247/550 | Category Version Number Report | Medium | Low |
| REQ-LLR-247-RE | I247/RE | Reserved Expansion Field | Medium | Low |
| REQ-LLR-247-SP | I247/SP | Special Purpose Field | Medium | Low |
| REQ-LLR-247-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-247-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 8

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-247
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-247-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT247 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
