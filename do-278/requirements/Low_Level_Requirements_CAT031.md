# Low-Level Requirements - ASTERIX Category 031
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT031-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-031
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 031 (Sensor Information Messages) v6.2.

**Parent Requirement:** REQ-HLR-CAT-031 - Parse ASTERIX Category 031

Each data item in CAT031 has corresponding LLRs defining parsing behavior.

---

## 2. CAT031 Data Item Requirements

### REQ-LLR-031-010: Parse Server Identification Tag (I031/010)

**Parent:** REQ-HLR-CAT-031
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I031/010 (Server Identification Tag) as specified in the EUROCONTROL ASTERIX CAT031 v6.2 specification.

**Data Item Definition:**
Identification of the Server of Sensor information.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-031-010-001: Parse valid Server Identification Tag
- TC-031-010-002: Verify format compliance
- TC-031-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-031-015: Parse User Number (I031/015)

**Parent:** REQ-HLR-CAT-031
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I031/015 (User Number) as specified in the EUROCONTROL ASTERIX CAT031 v6.2 specification.

**Data Item Definition:**
Identification of the User of Sensor information

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The User numbers are predefined in the User registration data base of the ARTAS Unit to which the User wants to connect.

**Verification Method:** Unit Test
**Test Cases:**
- TC-031-015-001: Parse valid User Number
- TC-031-015-002: Verify format compliance
- TC-031-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-031-020: Parse Time Of Message (I031/020)

**Parent:** REQ-HLR-CAT-031
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I031/020 (Time Of Message) as specified in the EUROCONTROL ASTERIX CAT031 v6.2 specification.

**Data Item Definition:**
Absolute time stamping of the message in the form of elapsed time since last midnight.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: This is the time at which a message is filled and not the time at which the data-block containing the tracks is sent. The time of the day value is reset to 0 at every midnight.

**Verification Method:** Unit Test
**Test Cases:**
- TC-031-020-001: Parse valid Time Of Message
- TC-031-020-002: Verify format compliance
- TC-031-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-031-030: Parse Sensor Identification Tag (I031/030)

**Parent:** REQ-HLR-CAT-031
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I031/030 (Sensor Identification Tag) as specified in the EUROCONTROL ASTERIX CAT031 v6.2 specification.

**Data Item Definition:**
Identification of the Sensor to which the provided information are related.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-031-030-001: Parse valid Sensor Identification Tag
- TC-031-030-002: Verify format compliance
- TC-031-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-031-040: Parse Sensor Status (I031/040)

**Parent:** REQ-HLR-CAT-031
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I031/040 (Sensor Status) as specified in the EUROCONTROL ASTERIX CAT031 v6.2 specification.

**Data Item Definition:**
Functioning status of the Sensor as monitored by ARTAS.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-031-040-001: Parse valid Sensor Status
- TC-031-040-002: Verify format compliance
- TC-031-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-031-050: Parse Time Stamping Bias (I031/050)

**Parent:** REQ-HLR-CAT-031
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I031/050 (Time Stamping Bias) as specified in the EUROCONTROL ASTERIX CAT031 v6.2 specification.

**Data Item Definition:**
Plot Time stamping bias

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-031-050-001: Parse valid Time Stamping Bias
- TC-031-050-002: Verify format compliance
- TC-031-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-031-060: Parse SSR Range Gain and Bias (I031/060)

**Parent:** REQ-HLR-CAT-031
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I031/060 (SSR Range Gain and Bias) as specified in the EUROCONTROL ASTERIX CAT031 v6.2 specification.

**Data Item Definition:**
SSR range gain and bias

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-031-060-001: Parse valid SSR Range Gain and Bias
- TC-031-060-002: Verify format compliance
- TC-031-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-031-070: Parse SSR Azimuth Bias (I031/070)

**Parent:** REQ-HLR-CAT-031
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I031/070 (SSR Azimuth Bias) as specified in the EUROCONTROL ASTERIX CAT031 v6.2 specification.

**Data Item Definition:**
SSR azimuth bias

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-031-070-001: Parse valid SSR Azimuth Bias
- TC-031-070-002: Verify format compliance
- TC-031-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-031-080: Parse PR Range Gain and Bias (I031/080)

**Parent:** REQ-HLR-CAT-031
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I031/080 (PR Range Gain and Bias) as specified in the EUROCONTROL ASTERIX CAT031 v6.2 specification.

**Data Item Definition:**
PR range gain and bias

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-031-080-001: Parse valid PR Range Gain and Bias
- TC-031-080-002: Verify format compliance
- TC-031-080-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-031-090: Parse PR Azimuth Bias (I031/090)

**Parent:** REQ-HLR-CAT-031
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I031/090 (PR Azimuth Bias) as specified in the EUROCONTROL ASTERIX CAT031 v6.2 specification.

**Data Item Definition:**
PR azimuth bias

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-031-090-001: Parse valid PR Azimuth Bias
- TC-031-090-002: Verify format compliance
- TC-031-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-031-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT031 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-031-ERR-001: Truncated data items
- TC-031-ERR-002: Invalid repetition counts
- TC-031-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-031-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT031 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-031-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-031-010 | I031/010 | Server Identification Tag | Medium | Low |
| REQ-LLR-031-015 | I031/015 | User Number | Medium | Low |
| REQ-LLR-031-020 | I031/020 | Time Of Message | Medium | Low |
| REQ-LLR-031-030 | I031/030 | Sensor Identification Tag | Medium | Low |
| REQ-LLR-031-040 | I031/040 | Sensor Status | Medium | Low |
| REQ-LLR-031-050 | I031/050 | Time Stamping Bias | Medium | Low |
| REQ-LLR-031-060 | I031/060 | SSR Range Gain and Bias | Medium | Low |
| REQ-LLR-031-070 | I031/070 | SSR Azimuth Bias | Medium | Low |
| REQ-LLR-031-080 | I031/080 | PR Range Gain and Bias | Medium | Low |
| REQ-LLR-031-090 | I031/090 | PR Azimuth Bias | Medium | Low |
| REQ-LLR-031-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-031-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 12

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-031
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-031-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT031 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
