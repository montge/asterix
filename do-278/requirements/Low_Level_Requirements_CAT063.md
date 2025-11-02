# Low-Level Requirements - ASTERIX Category 63
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT63-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-63
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 63 (Sensor Status Messages) v1.3.

**Parent Requirement:** REQ-HLR-CAT-63 - Parse ASTERIX Category 63

Each data item in CAT63 has corresponding LLRs defining parsing behavior.

---

## 2. CAT63 Data Item Requirements

### REQ-LLR-63-010: Parse Data Source Identifier (I63/010)

**Parent:** REQ-HLR-CAT-63
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I63/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT63 v1.3 specification.

**Data Item Definition:**
Identification of the SDPS sending the data

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-63-010-001: Parse valid Data Source Identifier
- TC-63-010-002: Verify format compliance
- TC-63-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-63-015: Parse Service Identification (I63/015)

**Parent:** REQ-HLR-CAT-63
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I63/015 (Service Identification) as specified in the EUROCONTROL ASTERIX CAT63 v1.3 specification.

**Data Item Definition:**
Identification of the service provided to one or more users.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: the service identification is allocated by the SDPS

**Verification Method:** Unit Test
**Test Cases:**
- TC-63-015-001: Parse valid Service Identification
- TC-63-015-002: Verify format compliance
- TC-63-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-63-030: Parse Time Of Message (I63/030)

**Parent:** REQ-HLR-CAT-63
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I63/030 (Time Of Message) as specified in the EUROCONTROL ASTERIX CAT63 v1.3 specification.

**Data Item Definition:**
Absolute time stamping of the message in the form of elapsed time since last midnight.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: The time of the day value is reset to 0 at every midnight.

**Verification Method:** Unit Test
**Test Cases:**
- TC-63-030-001: Parse valid Time Of Message
- TC-63-030-002: Verify format compliance
- TC-63-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-63-050: Parse Sensor Identifier (I63/050)

**Parent:** REQ-HLR-CAT-63
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I63/050 (Sensor Identifier) as specified in the EUROCONTROL ASTERIX CAT63 v1.3 specification.

**Data Item Definition:**
Identification of the Sensor to which the provided information are related.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-63-050-001: Parse valid Sensor Identifier
- TC-63-050-002: Verify format compliance
- TC-63-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-63-060: Parse Sensor Configuration and Status (I63/060)

**Parent:** REQ-HLR-CAT-63
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I63/060 (Sensor Configuration and Status) as specified in the EUROCONTROL ASTERIX CAT63 v1.3 specification.

**Data Item Definition:**
Configuration and status of the sensor

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-63-060-001: Parse valid Sensor Configuration and Status
- TC-63-060-002: Verify format compliance
- TC-63-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-63-070: Parse Time Stamping Bias (I63/070)

**Parent:** REQ-HLR-CAT-63
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I63/070 (Time Stamping Bias) as specified in the EUROCONTROL ASTERIX CAT63 v1.3 specification.

**Data Item Definition:**
Plot Time stamping bias

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-63-070-001: Parse valid Time Stamping Bias
- TC-63-070-002: Verify format compliance
- TC-63-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-63-080: Parse SSR/ModeS Range Gain and Bias (I63/080)

**Parent:** REQ-HLR-CAT-63
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I63/080 (SSR/ModeS Range Gain and Bias) as specified in the EUROCONTROL ASTERIX CAT63 v1.3 specification.

**Data Item Definition:**
SSR / Mode S Range Gain and Range Bias, in two’s complement form.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-63-080-001: Parse valid SSR/ModeS Range Gain and Bias
- TC-63-080-002: Verify format compliance
- TC-63-080-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-63-081: Parse SSR/ModeS Azimuth Bias (I63/081)

**Parent:** REQ-HLR-CAT-63
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I63/081 (SSR/ModeS Azimuth Bias) as specified in the EUROCONTROL ASTERIX CAT63 v1.3 specification.

**Data Item Definition:**
SSR / Mode S Azimuth Bias, in two’s complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-63-081-001: Parse valid SSR/ModeS Azimuth Bias
- TC-63-081-002: Verify format compliance
- TC-63-081-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-63-090: Parse PSR Range Gain and Bias (I63/090)

**Parent:** REQ-HLR-CAT-63
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I63/090 (PSR Range Gain and Bias) as specified in the EUROCONTROL ASTERIX CAT63 v1.3 specification.

**Data Item Definition:**
PSR Range Gain and PSR Range Bias, in two’s complement form.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-63-090-001: Parse valid PSR Range Gain and Bias
- TC-63-090-002: Verify format compliance
- TC-63-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-63-091: Parse PSR Azimuth Bias (I63/091)

**Parent:** REQ-HLR-CAT-63
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I63/091 (PSR Azimuth Bias) as specified in the EUROCONTROL ASTERIX CAT63 v1.3 specification.

**Data Item Definition:**
PSR Azimuth Bias, in two’s complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-63-091-001: Parse valid PSR Azimuth Bias
- TC-63-091-002: Verify format compliance
- TC-63-091-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-63-092: Parse PSR Elevation Bias (I63/092)

**Parent:** REQ-HLR-CAT-63
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I63/092 (PSR Elevation Bias) as specified in the EUROCONTROL ASTERIX CAT63 v1.3 specification.

**Data Item Definition:**
PSR Elevation Bias, in two’s complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-63-092-001: Parse valid PSR Elevation Bias
- TC-63-092-002: Verify format compliance
- TC-63-092-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-63-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT63 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-63-ERR-001: Truncated data items
- TC-63-ERR-002: Invalid repetition counts
- TC-63-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-63-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT63 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-63-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-63-010 | I63/010 | Data Source Identifier | Medium | Low |
| REQ-LLR-63-015 | I63/015 | Service Identification | Medium | Low |
| REQ-LLR-63-030 | I63/030 | Time Of Message | Medium | Low |
| REQ-LLR-63-050 | I63/050 | Sensor Identifier | Medium | Low |
| REQ-LLR-63-060 | I63/060 | Sensor Configuration and Status | Medium | Low |
| REQ-LLR-63-070 | I63/070 | Time Stamping Bias | Medium | Low |
| REQ-LLR-63-080 | I63/080 | SSR/ModeS Range Gain and Bias | Medium | Low |
| REQ-LLR-63-081 | I63/081 | SSR/ModeS Azimuth Bias | Medium | Low |
| REQ-LLR-63-090 | I63/090 | PSR Range Gain and Bias | Medium | Low |
| REQ-LLR-63-091 | I63/091 | PSR Azimuth Bias | Medium | Low |
| REQ-LLR-63-092 | I63/092 | PSR Elevation Bias | Medium | Low |
| REQ-LLR-63-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-63-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 13

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-63
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-63-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT63 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
