# Low-Level Requirements - ASTERIX Category 31
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT31-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-31
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 31 (Sensor Information Messages) v6.2.

**Parent Requirement:** REQ-HLR-CAT-31 - Parse ASTERIX Category 31

Each data item in CAT31 has corresponding LLRs defining parsing behavior.

---

## 2. CAT31 Data Item Requirements

### REQ-LLR-31-010: Parse Server Identification Tag (I31/010)

**Parent:** REQ-HLR-CAT-31
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I31/010 (Server Identification Tag) as specified in the EUROCONTROL ASTERIX CAT31 v6.2 specification.

**Data Item Definition:**
Identification of the Server of Sensor information.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-31-010-001: Parse valid Server Identification Tag
- TC-31-010-002: Verify format compliance
- TC-31-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-31-015: Parse User Number (I31/015)

**Parent:** REQ-HLR-CAT-31
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I31/015 (User Number) as specified in the EUROCONTROL ASTERIX CAT31 v6.2 specification.

**Data Item Definition:**
Identification of the User of Sensor information

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The User numbers are predefined in the User registration data base of the ARTAS Unit to which the User wants to connect.

**Verification Method:** Unit Test
**Test Cases:**
- TC-31-015-001: Parse valid User Number
- TC-31-015-002: Verify format compliance
- TC-31-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-31-020: Parse Time Of Message (I31/020)

**Parent:** REQ-HLR-CAT-31
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I31/020 (Time Of Message) as specified in the EUROCONTROL ASTERIX CAT31 v6.2 specification.

**Data Item Definition:**
Absolute time stamping of the message in the form of elapsed time since last midnight.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: This is the time at which a message is filled and not the time at which the data-block containing the tracks is sent. The time of the day value is reset to 0 at every midnight.

**Verification Method:** Unit Test
**Test Cases:**
- TC-31-020-001: Parse valid Time Of Message
- TC-31-020-002: Verify format compliance
- TC-31-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-31-030: Parse Sensor Identification Tag (I31/030)

**Parent:** REQ-HLR-CAT-31
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I31/030 (Sensor Identification Tag) as specified in the EUROCONTROL ASTERIX CAT31 v6.2 specification.

**Data Item Definition:**
Identification of the Sensor to which the provided information are related.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-31-030-001: Parse valid Sensor Identification Tag
- TC-31-030-002: Verify format compliance
- TC-31-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-31-040: Parse Sensor Status (I31/040)

**Parent:** REQ-HLR-CAT-31
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I31/040 (Sensor Status) as specified in the EUROCONTROL ASTERIX CAT31 v6.2 specification.

**Data Item Definition:**
Functioning status of the Sensor as monitored by ARTAS.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-31-040-001: Parse valid Sensor Status
- TC-31-040-002: Verify format compliance
- TC-31-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-31-050: Parse Time Stamping Bias (I31/050)

**Parent:** REQ-HLR-CAT-31
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I31/050 (Time Stamping Bias) as specified in the EUROCONTROL ASTERIX CAT31 v6.2 specification.

**Data Item Definition:**
Plot Time stamping bias

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-31-050-001: Parse valid Time Stamping Bias
- TC-31-050-002: Verify format compliance
- TC-31-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-31-060: Parse SSR Range Gain and Bias (I31/060)

**Parent:** REQ-HLR-CAT-31
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I31/060 (SSR Range Gain and Bias) as specified in the EUROCONTROL ASTERIX CAT31 v6.2 specification.

**Data Item Definition:**
SSR range gain and bias

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-31-060-001: Parse valid SSR Range Gain and Bias
- TC-31-060-002: Verify format compliance
- TC-31-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-31-070: Parse SSR Azimuth Bias (I31/070)

**Parent:** REQ-HLR-CAT-31
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I31/070 (SSR Azimuth Bias) as specified in the EUROCONTROL ASTERIX CAT31 v6.2 specification.

**Data Item Definition:**
SSR azimuth bias

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-31-070-001: Parse valid SSR Azimuth Bias
- TC-31-070-002: Verify format compliance
- TC-31-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-31-080: Parse PR Range Gain and Bias (I31/080)

**Parent:** REQ-HLR-CAT-31
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I31/080 (PR Range Gain and Bias) as specified in the EUROCONTROL ASTERIX CAT31 v6.2 specification.

**Data Item Definition:**
PR range gain and bias

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-31-080-001: Parse valid PR Range Gain and Bias
- TC-31-080-002: Verify format compliance
- TC-31-080-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-31-090: Parse PR Azimuth Bias (I31/090)

**Parent:** REQ-HLR-CAT-31
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I31/090 (PR Azimuth Bias) as specified in the EUROCONTROL ASTERIX CAT31 v6.2 specification.

**Data Item Definition:**
PR azimuth bias

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-31-090-001: Parse valid PR Azimuth Bias
- TC-31-090-002: Verify format compliance
- TC-31-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat031_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-31-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT31 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-31-ERR-001: Truncated data items
- TC-31-ERR-002: Invalid repetition counts
- TC-31-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-31-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT31 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-31-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-31-010 | I31/010 | Server Identification Tag | Medium | Low |
| REQ-LLR-31-015 | I31/015 | User Number | Medium | Low |
| REQ-LLR-31-020 | I31/020 | Time Of Message | Medium | Low |
| REQ-LLR-31-030 | I31/030 | Sensor Identification Tag | Medium | Low |
| REQ-LLR-31-040 | I31/040 | Sensor Status | Medium | Low |
| REQ-LLR-31-050 | I31/050 | Time Stamping Bias | Medium | Low |
| REQ-LLR-31-060 | I31/060 | SSR Range Gain and Bias | Medium | Low |
| REQ-LLR-31-070 | I31/070 | SSR Azimuth Bias | Medium | Low |
| REQ-LLR-31-080 | I31/080 | PR Range Gain and Bias | Medium | Low |
| REQ-LLR-31-090 | I31/090 | PR Azimuth Bias | Medium | Low |
| REQ-LLR-31-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-31-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 12

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-31
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-31-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT31 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
