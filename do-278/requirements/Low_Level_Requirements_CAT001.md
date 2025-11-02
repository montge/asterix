# Low-Level Requirements - ASTERIX Category 1
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT1-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-1
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 1 (Transmission of Monoroadar Data) v1.2.

**Parent Requirement:** REQ-HLR-CAT-1 - Parse ASTERIX Category 1

Each data item in CAT1 has corresponding LLRs defining parsing behavior.

---

## 2. CAT1 Data Item Requirements

### REQ-LLR-1-010: Parse Data Source Identifier (I1/010)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I1/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Identification of the radar station from which the data are received.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-010-001: Parse valid Data Source Identifier
- TC-1-010-002: Verify format compliance
- TC-1-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-1-020: Parse Target Report Descriptor (I1/020)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I1/020 (Target Report Descriptor) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Type and characteristics of the radar data as transmitted by a radar station.

**Implementation Notes:**
- Format: Variable
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-020-001: Parse valid Target Report Descriptor
- TC-1-020-002: Verify format compliance
- TC-1-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-1-030: Parse Warning/Error Conditions (I1/030)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I1/030 (Warning/Error Conditions) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Warning/error conditions detected by a radar station for the target report involved.

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-030-001: Parse valid Warning/Error Conditions
- TC-1-030-002: Verify format compliance
- TC-1-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-1-040: Parse Measured Position in Polar Coordinates (I1/040)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I1/040 (Measured Position in Polar Coordinates) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Measured position of an aircraft in local polar coordinates.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-040-001: Parse valid Measured Position in Polar Coordinates
- TC-1-040-002: Verify format compliance
- TC-1-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-1-042: Parse Calculated Position in Cartesian Coordinates (I1/042)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I1/042 (Calculated Position in Cartesian Coordinates) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Calculated position of an aircraft in cartesian coordinates

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: Scaling is 2^(-6+f) where f is scalling factor

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-042-001: Parse valid Calculated Position in Cartesian Coordinates
- TC-1-042-002: Verify format compliance
- TC-1-042-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-1-050: Parse Mode-2 Code (I1/050)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I1/050 (Mode-2 Code) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Reply to Mode-2 interrogation

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: Smoothed Mode-2 data (bit-14 set to one) is used when the plot contains no Mode-2 code or the Mode-2 codes of the plot and track are different. 2. Bits-16/15 have no meaning in the case of a smoothed Mode-2 and are set to 0 for a calculated track.

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-050-001: Parse valid Mode-2 Code
- TC-1-050-002: Verify format compliance
- TC-1-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-1-060: Parse Mode-2 Code Confidence Indicator (I1/060)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I1/060 (Mode-2 Code Confidence Indicator) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Confidence level for each bit of a Mode-2 reply as provided by a monopulse SSR station.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: This Data Item is only transmitted if at least one pulse is of low quality.

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-060-001: Parse valid Mode-2 Code Confidence Indicator
- TC-1-060-002: Verify format compliance
- TC-1-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-1-070: Parse Mode-3/a Code (I1/070)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I1/070 (Mode-3/a Code) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Mode-3/A code converted into octal representation.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: mandatory
- Note: The detector signals a garbled code (bit-15 set to one) when at least two replies are overlapping. Smoothed Mode-3/A data (bit-14 set to a one) are used in the case of the absence of Mode-3/A code information in the plot, or in the case of a difference between the plot and track Mode-3/A code information. Bits-16/15 have no meaning in the case of a smoothed Mode-3/A and are set to 0 for a calculated track.

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-070-001: Parse valid Mode-3/a Code
- TC-1-070-002: Verify format compliance
- TC-1-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-1-080: Parse Mode-3/A Code Confidence Indicator (I1/080)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I1/080 (Mode-3/A Code Confidence Indicator) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Confidence level for each bit of a Mode-3/A reply as provided by a monopulse SSR station.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: This Data Item is only transmitted if at least one pulse is of low quality.

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-080-001: Parse valid Mode-3/A Code Confidence Indicator
- TC-1-080-002: Verify format compliance
- TC-1-080-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-1-090: Parse Mode-C Code (I1/090)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I1/090 (Mode-C Code) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Mode-C height converted into binary representation

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: mandatory
- Note: The detector signals a garbled code when at least two replies are overlapping. 2. The maximum height which can be represented is 204 775 ft. Practically the maximum valid value is 126 750 ft (refer to ICAO Annex 10). 3. Negative values are expressed in 2's complement form, bit-14 is set to 0 for positive values and 1 for negative values.

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-090-001: Parse valid Mode-C Code
- TC-1-090-002: Verify format compliance
- TC-1-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-1-100: Parse Mode-C Code and Code Confidence Indicator (I1/100)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I1/100 (Mode-C Code and Code Confidence Indicator) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Mode-C height in Gray notation as received from the transponder together with the confidence level for each reply bit as provided by a monopulse SSR station.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: This Data Item is only transmitted if at least one pulse is of low quality. 2. The detector signals a garbled code when at least two replies are overlapping.

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-100-001: Parse valid Mode-C Code and Code Confidence Indicator
- TC-1-100-002: Verify format compliance
- TC-1-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-1-120: Parse Measured Radial Doppler Speed (I1/120)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I1/120 (Measured Radial Doppler Speed) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Radial component of the ground speed as measured by means of Doppler filter banks in radar signal processors.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: A default quantisation unit of 14.062 5 kt and a maximum of +/- 1 800 kt is obtained for a value of f = 6. 2. Negative values are expressed in 2's complement form, bit-8 is set to 0 for positive values and 1 for negative values.

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-120-001: Parse valid Measured Radial Doppler Speed
- TC-1-120-002: Verify format compliance
- TC-1-120-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-1-130: Parse Radar Plot Characteristics (I1/130)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I1/130 (Radar Plot Characteristics) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Additional information on the quality of the target report

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: This Data Item may contain parameters such as plot runlength (primary and secondary), difference between primary and secondary derived azimuth, pulse amplitude, etc.

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-130-001: Parse valid Radar Plot Characteristics
- TC-1-130-002: Verify format compliance
- TC-1-130-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-1-131: Parse Received Power (I1/131)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I1/131 (Received Power) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Measurement of the received power.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: POWER is the measured value of the power received on the sum pattern for a plot. 2. Negative values are expressed in 2's complement form, bit-8 is set to 0 for positive values and 1 for negative values.

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-131-001: Parse valid Received Power
- TC-1-131-002: Verify format compliance
- TC-1-131-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-1-141: Parse Truncated Time of Day (I1/141)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I1/141 (Truncated Time of Day) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Absolute time stamping expressed as Coordinated Universal Time (UTC) time.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The exchange of this Data Item allows the easy derivation of the correct UTC time value, provided that the clocks at the data source and sink(s) are less than 512 seconds out of synchronisation. Special care has to be taken at the transition of an "all ones" value to an "all zeros" value (every 512 seconds). 2. The time of day value is reset to 0 each day at midnight. 3. For time management in radar transmission applications, refer to Part 1, paragraph 5.4 [Ref. 2].

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-141-001: Parse valid Truncated Time of Day
- TC-1-141-002: Verify format compliance
- TC-1-141-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-1-150: Parse Presence of X-Pulse (I1/150)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I1/150 (Presence of X-Pulse) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Presence of the X-Pulse for the various modes applied in the interrogation interlace pattern.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: This Data Item is transmitted only if at least one X-pulse has been received in a Mode-A, Mode-2 or Mode-C reply.

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-150-001: Parse valid Presence of X-Pulse
- TC-1-150-002: Verify format compliance
- TC-1-150-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-1-161: Parse Track Plot Number (I1/161)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I1/161 (Track Plot Number) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
An integer value representing a unique reference to a track/plot record within a particular track/plot file.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: mandatory
- Note: The differentiation between track and plot number is either implicit or is made via the Target Report Descriptor (Data Item I001/020).

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-161-001: Parse valid Track Plot Number
- TC-1-161-002: Verify format compliance
- TC-1-161-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-1-170: Parse Track Status (I1/170)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I1/170 (Track Status) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Status of track derived either from primary and/or secondary radar information.

**Implementation Notes:**
- Format: Variable
- Rule: mandatory
- Note: Bit-2 (GHO) is used to signal that the track is suspected to have been generated by a fake target. 2. Bit-4 (RDPC) is used to signal the discontinuity of the track numbers.

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-170-001: Parse valid Track Status
- TC-1-170-002: Verify format compliance
- TC-1-170-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-1-200: Parse Calculated Track Velocity in Polar Coordinates (I1/200)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I1/200 (Calculated Track Velocity in Polar Coordinates) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Calculated track velocity expressed in polar coordinates.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-200-001: Parse valid Calculated Track Velocity in Polar Coordinates
- TC-1-200-002: Verify format compliance
- TC-1-200-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-1-210: Parse Track Quality (I1/210)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I1/210 (Track Quality) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Relative track quality.

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: Actual bit signification is application dependent

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-210-001: Parse valid Track Quality
- TC-1-210-002: Verify format compliance
- TC-1-210-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-1-SP: Parse Special Purpose (I1/SP)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I1/SP (Special Purpose) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
Special information

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-SP-001: Parse valid Special Purpose
- TC-1-SP-002: Verify format compliance
- TC-1-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-1-RFS: Parse RFS Indicator (I1/RFS)

**Parent:** REQ-HLR-CAT-1
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I1/RFS (RFS Indicator) as specified in the EUROCONTROL ASTERIX CAT1 v1.2 specification.

**Data Item Definition:**
RFS Indicator

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-RFS-001: Parse valid RFS Indicator
- TC-1-RFS-002: Verify format compliance
- TC-1-RFS-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-1-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT1 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-ERR-001: Truncated data items
- TC-1-ERR-002: Invalid repetition counts
- TC-1-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-1-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT1 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-1-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-1-010 | I1/010 | Data Source Identifier | Critical | High |
| REQ-LLR-1-020 | I1/020 | Target Report Descriptor | Critical | High |
| REQ-LLR-1-030 | I1/030 | Warning/Error Conditions | Medium | Low |
| REQ-LLR-1-040 | I1/040 | Measured Position in Polar Coordinates | Critical | High |
| REQ-LLR-1-042 | I1/042 | Calculated Position in Cartesian Coordin | Medium | Low |
| REQ-LLR-1-050 | I1/050 | Mode-2 Code | Medium | Low |
| REQ-LLR-1-060 | I1/060 | Mode-2 Code Confidence Indicator | Medium | Low |
| REQ-LLR-1-070 | I1/070 | Mode-3/a Code | Critical | High |
| REQ-LLR-1-080 | I1/080 | Mode-3/A Code Confidence Indicator | Medium | Low |
| REQ-LLR-1-090 | I1/090 | Mode-C Code | Critical | High |
| REQ-LLR-1-100 | I1/100 | Mode-C Code and Code Confidence Indicato | Medium | Low |
| REQ-LLR-1-120 | I1/120 | Measured Radial Doppler Speed | Medium | Low |
| REQ-LLR-1-130 | I1/130 | Radar Plot Characteristics | Medium | Low |
| REQ-LLR-1-131 | I1/131 | Received Power | Medium | Low |
| REQ-LLR-1-141 | I1/141 | Truncated Time of Day | Medium | Low |
| REQ-LLR-1-150 | I1/150 | Presence of X-Pulse | Medium | Low |
| REQ-LLR-1-161 | I1/161 | Track Plot Number | Critical | High |
| REQ-LLR-1-170 | I1/170 | Track Status | Critical | High |
| REQ-LLR-1-200 | I1/200 | Calculated Track Velocity in Polar Coord | Medium | Low |
| REQ-LLR-1-210 | I1/210 | Track Quality | Medium | Low |
| REQ-LLR-1-SP | I1/SP | Special Purpose | Medium | Low |
| REQ-LLR-1-RFS | I1/RFS | RFS Indicator | Medium | Low |
| REQ-LLR-1-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-1-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 24

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-1
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-1-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT1 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
