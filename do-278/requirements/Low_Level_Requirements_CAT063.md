# Low-Level Requirements - ASTERIX Category 063
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT063-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-063
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 063 (Sensor Status Reports) v1.6.

**Parent Requirement:** REQ-HLR-CAT-063 - Parse ASTERIX Category 063

Each data item in CAT063 has corresponding LLRs defining parsing behavior.

---

## 2. CAT063 Data Item Requirements

### REQ-LLR-063-010: Parse Data Source Identifier (I063/010)

**Parent:** REQ-HLR-CAT-063
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I063/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT063 v1.6 specification.

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
- TC-063-010-001: Parse valid Data Source Identifier
- TC-063-010-002: Verify format compliance
- TC-063-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-063-015: Parse Service Identification (I063/015)

**Parent:** REQ-HLR-CAT-063
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I063/015 (Service Identification) as specified in the EUROCONTROL ASTERIX CAT063 v1.6 specification.

**Data Item Definition:**

            Identification of the service provided to one or more users.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
            The service identification is allocated by the SDPS
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-063-015-001: Parse valid Service Identification
- TC-063-015-002: Verify format compliance
- TC-063-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-063-030: Parse Time of Message (I063/030)

**Parent:** REQ-HLR-CAT-063
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I063/030 (Time of Message) as specified in the EUROCONTROL ASTERIX CAT063 v1.6 specification.

**Data Item Definition:**

            Absolute time stamping of the message, in the form of elapsed time
            since last midnight, expressed as UTC.
        

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: 
            The time of the day value is reset to zero at every midnight.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-063-030-001: Parse valid Time of Message
- TC-063-030-002: Verify format compliance
- TC-063-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-063-050: Parse Sensor Identifier (I063/050)

**Parent:** REQ-HLR-CAT-063
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I063/050 (Sensor Identifier) as specified in the EUROCONTROL ASTERIX CAT063 v1.6 specification.

**Data Item Definition:**


**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            - The up-to-date list of SACs is published on the EUROCONTROL
              Web Site (http://www.eurocontrol.int/asterix).
            - If the SAC/SIC refers to an SDPS used as input, the respective
              sensor status information will be transmitted using the Reserved
              Expansion Field.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-063-050-001: Parse valid Sensor Identifier
- TC-063-050-002: Verify format compliance
- TC-063-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-063-060: Parse Sensor Configuration and Status (I063/060)

**Parent:** REQ-HLR-CAT-063
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I063/060 (Sensor Configuration and Status) as specified in the EUROCONTROL ASTERIX CAT063 v1.6 specification.

**Data Item Definition:**

            Configuration and status of the sensor
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            1. GO/NOGO information from PSR, SSR, Mode S, ADS and MLT is derived
               from monosensor categories and has a meaning only for operational
               sensors, whereas (CON) is derived by the SDPS.
            2. The information (OPS), (ODP), (OXT), (MSC) and (TSV) are only related to
               CNS/ATM Ground Station and are derived from monosensor category
               (ASTERIX Cat 023).
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-063-060-001: Parse valid Sensor Configuration and Status
- TC-063-060-002: Verify format compliance
- TC-063-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-063-070: Parse Time Stamping Bias (I063/070)

**Parent:** REQ-HLR-CAT-063
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I063/070 (Time Stamping Bias) as specified in the EUROCONTROL ASTERIX CAT063 v1.6 specification.

**Data Item Definition:**

            Plot Time stamping bias, in two's complement form
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-063-070-001: Parse valid Time Stamping Bias
- TC-063-070-002: Verify format compliance
- TC-063-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-063-080: Parse SSR / Mode S Range Gain and Bias (I063/080)

**Parent:** REQ-HLR-CAT-063
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I063/080 (SSR / Mode S Range Gain and Bias) as specified in the EUROCONTROL ASTERIX CAT063 v1.6 specification.

**Data Item Definition:**

            SSR / Mode S Range Gain and Range Bias, in two's complement form.
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: 
            Note:

                The following formula is used to correct range:

                .. math::

                    \rho_\mathrm{corrected} = \frac{\rho_\mathrm{measured} - range\_bias}{1 + range\_gain}
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-063-080-001: Parse valid SSR / Mode S Range Gain and Bias
- TC-063-080-002: Verify format compliance
- TC-063-080-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-063-081: Parse SSR Mode S Azimuth Bias (I063/081)

**Parent:** REQ-HLR-CAT-063
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I063/081 (SSR Mode S Azimuth Bias) as specified in the EUROCONTROL ASTERIX CAT063 v1.6 specification.

**Data Item Definition:**

            SSR / Mode S Azimuth Bias, in two's complement form.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Note:

                The following formula is used to correct azimuth:

                .. math::

                    \theta_\mathrm{corrected} = \theta_\mathrm{measured} - azimuth\_bias
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-063-081-001: Parse valid SSR Mode S Azimuth Bias
- TC-063-081-002: Verify format compliance
- TC-063-081-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-063-090: Parse PSR Range Gain and Bias (I063/090)

**Parent:** REQ-HLR-CAT-063
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I063/090 (PSR Range Gain and Bias) as specified in the EUROCONTROL ASTERIX CAT063 v1.6 specification.

**Data Item Definition:**

            PSR Range Gain and PSR Range Bias, in two's complement form.
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: 
            Note:

                The following formula is used to correct range:

                .. math:

                    \rho_\mathrm{corrected} = \frac{\rho_\mathrm{measured} - range\_bias}{1 + range\_gain}
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-063-090-001: Parse valid PSR Range Gain and Bias
- TC-063-090-002: Verify format compliance
- TC-063-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-063-091: Parse PSR Azimuth Bias (I063/091)

**Parent:** REQ-HLR-CAT-063
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I063/091 (PSR Azimuth Bias) as specified in the EUROCONTROL ASTERIX CAT063 v1.6 specification.

**Data Item Definition:**

            PSR Azimuth Bias, in two's complement form.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Note:

                The following formula is used to correct azimuth:

                .. math::

                    \theta_\mathrm{corrected} = \theta_\mathrm{measured} - azimuth\_bias
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-063-091-001: Parse valid PSR Azimuth Bias
- TC-063-091-002: Verify format compliance
- TC-063-091-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-063-092: Parse PSR Elevation Bias (I063/092)

**Parent:** REQ-HLR-CAT-063
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I063/092 (PSR Elevation Bias) as specified in the EUROCONTROL ASTERIX CAT063 v1.6 specification.

**Data Item Definition:**

            PSR Elevation Bias, in two's complement form.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-063-092-001: Parse valid PSR Elevation Bias
- TC-063-092-002: Verify format compliance
- TC-063-092-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-063-RE: Parse Reserved Expansion Field (I063/RE)

**Parent:** REQ-HLR-CAT-063
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I063/RE (Reserved Expansion Field) as specified in the EUROCONTROL ASTERIX CAT063 v1.6 specification.

**Data Item Definition:**

            Expansion
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-063-RE-001: Parse valid Reserved Expansion Field
- TC-063-RE-002: Verify format compliance
- TC-063-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-063-SP: Parse Special Purpose Field (I063/SP)

**Parent:** REQ-HLR-CAT-063
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I063/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT063 v1.6 specification.

**Data Item Definition:**

            Special Purpose Field
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-063-SP-001: Parse valid Special Purpose Field
- TC-063-SP-002: Verify format compliance
- TC-063-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat063_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-063-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT063 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-063-ERR-001: Truncated data items
- TC-063-ERR-002: Invalid repetition counts
- TC-063-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-063-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT063 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-063-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-063-010 | I063/010 | Data Source Identifier | Medium | Low |
| REQ-LLR-063-015 | I063/015 | Service Identification | Medium | Low |
| REQ-LLR-063-030 | I063/030 | Time of Message | Medium | Low |
| REQ-LLR-063-050 | I063/050 | Sensor Identifier | Medium | Low |
| REQ-LLR-063-060 | I063/060 | Sensor Configuration and Status | Medium | Low |
| REQ-LLR-063-070 | I063/070 | Time Stamping Bias | Medium | Low |
| REQ-LLR-063-080 | I063/080 | SSR / Mode S Range Gain and Bias | Medium | Low |
| REQ-LLR-063-081 | I063/081 | SSR Mode S Azimuth Bias | Medium | Low |
| REQ-LLR-063-090 | I063/090 | PSR Range Gain and Bias | Medium | Low |
| REQ-LLR-063-091 | I063/091 | PSR Azimuth Bias | Medium | Low |
| REQ-LLR-063-092 | I063/092 | PSR Elevation Bias | Medium | Low |
| REQ-LLR-063-RE | I063/RE | Reserved Expansion Field | Medium | Low |
| REQ-LLR-063-SP | I063/SP | Special Purpose Field | Medium | Low |
| REQ-LLR-063-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-063-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 15

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-063
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-063-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT063 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
