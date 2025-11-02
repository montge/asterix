# Low-Level Requirements - ASTERIX Category 4
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT4-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-4
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 4 (Safety Net Messages) v1.12.

**Parent Requirement:** REQ-HLR-CAT-4 - Parse ASTERIX Category 4

Each data item in CAT4 has corresponding LLRs defining parsing behavior.

---

## 2. CAT4 Data Item Requirements

### REQ-LLR-4-000: Parse Message Type (I4/000)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I4/000 (Message Type) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
This Data Item allows for a more convenient handling of the messages at the receiver side by further defining the type of transaction.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: mandatory
- Note: 1. In applications where transactions of various types are exchanged, the Message Type Data Item facilitates the proper message handling at the receiver side. 2. All Message Type values are reserved for common standard use.

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-000-001: Parse valid Message Type
- TC-4-000-002: Verify format compliance
- TC-4-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-4-010: Parse Data Source Identifier (I4/010)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I4/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Identification of the Safety Nets server sending the message.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: mandatory
- Note: The up-to-date list of SACs is published on the EUROCONTROL Web Site (http://www.eurocontrol.int/asterix).

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-010-001: Parse valid Data Source Identifier
- TC-4-010-002: Verify format compliance
- TC-4-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-4-015: Parse SDPS Identifier (I4/015)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/015 (SDPS Identifier) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Identification of the SDPS providing data to the safety nets server.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: The up-to-date list of SACs is published on the Eurocontrol Web Site (http://www.eurocontrol.int/asterix).

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-015-001: Parse valid SDPS Identifier
- TC-4-015-002: Verify format compliance
- TC-4-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-020: Parse Time of Message (I4/020)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I4/020 (Time of Message) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Absolute time stamping of the message in the form of elapsed time since last midnight

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: mandatory
- Note: This time is given at an application level (e.g. time at which a message is filled), and not at the communication level (i.e. not the time at which the datablock containing the message is sent).

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-020-001: Parse valid Time of Message
- TC-4-020-002: Verify format compliance
- TC-4-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-4-030: Parse Track Number 1 (I4/030)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/030 (Track Number 1) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Identification of a track number related to conflict

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
		1. This is the track number of the first track involved in the conflict in case of an STCA or a RIMCA or the track involved in case of one of the other Safety Net functions. 
		2. This track number is distributed in this field exactly as it was received from the Radar Processor Unit (identified by I004/015) and its range is depending on the range used by that unit. 
		3. In case of Message Type = 33 (Final Target Distance Indicator - FTD) this represents the Track Number of the following aircraft. 
		4. In case of Message Type = 34 (Initial Target Distance Indicator - ITD) this represents the Track Number of the following aircraft. 
		5. In case of Message Type = 35 (Wake Vortex Indicator Infringement Alert - IIA) this represents the track number of the following aircraft. 
		6. In case of Message Type = 37 (Catch-Up Warning - CUW) this represents the track number of the following aircraft (i.e. the one catching up).

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-030-001: Parse valid Track Number 1
- TC-4-030-002: Verify format compliance
- TC-4-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-035: Parse Track Number 2 (I4/035)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/035 (Track Number 2) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Together with I004/030, this item defines the track pair in conflict.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 1. This is the track number of the second track involved in the conflict in case of an STCA, a RIMCA, a NTCA, a VCD or in message types 33 to 35 and 37.
		2. For the other Safety Net functions, this item is not used.
		3. This track number is distributed in this field exactly as it was received from the Radar Processor Unit and its range is depending on the range used by that unit.
		4. In case of Message Type = 33 (Final Target Distance Indicator - FTD) this represents the Track Number of the leading aircraft
		5. In case of Message Type = 34 (Initial Target Distance Indicator - ITD) this represents the Track Number of the leading aircraft.
		6. In case of Message Type = 35 (Wake Vortex Indicator Infringement Alert - IIA) this represents the track number of the leading aircraft.
		7. In case of Message Type = 37 (Catch-Up Warning - CUW) this represents the track number of the leading aircraft.
		8. In case of Message Type = 38 (Conflicting ATC Clearances - CATC) this represents the track number of the aircraft to which the first ATC	Clearance was issued.

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-035-001: Parse valid Track Number 2
- TC-4-035-002: Verify format compliance
- TC-4-035-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-040: Parse Alert Identifier (I4/040)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/040 (Alert Identifier) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Identification of an alert (Alert number)

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 1. This item is the Alert Identification of the conflict in the system 2. This number shall be assigned, by the Safety Net Server, incrementally to every new alert and restart on zero after reaching the maximum value (65535)

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-040-001: Parse valid Alert Identifier
- TC-4-040-002: Verify format compliance
- TC-4-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-045: Parse Alert Status (I4/045)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/045 (Alert Status) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Information concerning status of the alert

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: The content of this item is implementation dependent.

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-045-001: Parse valid Alert Status
- TC-4-045-002: Verify format compliance
- TC-4-045-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-060: Parse Safety Net Function and System Status (I4/060)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/060 (Safety Net Function and System Status) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Status of the Safety Nets functions handled by the system

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 1. This item only sent in “alive messages” to describe the status of the Safety Net functions, handled by the system
		2. Value 0 means either that the function is not managed by the system or has failed.
		3. Value 1 means that the function is managed by the system and is running well
		4. “Overflow” is defined as a situation where the number of alerts in the system has exceeded the threshold for safe operation. Potential prioritization of the alerts may lead to a loss of information.
		5. “Overload” is defined as a system status in which the number of alerts does not allow for a reliable performance. A correct calculation and transmission cannot be guaranteed.
		6. “System degraded” means that information from one or more sensors is lost. 

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-060-001: Parse valid Safety Net Function and System Status
- TC-4-060-002: Verify format compliance
- TC-4-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-070: Parse Conflict Timing and Separation (I4/070)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/070 (Conflict Timing and Separation) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Information on Timing and Aircraft Separation

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-070-001: Parse valid Conflict Timing and Separation
- TC-4-070-002: Verify format compliance
- TC-4-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-074: Parse Longitudinal Deviation (I4/074)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/074 (Longitudinal Deviation) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Longitudinal deviation for Route Adherence Monitoring, in two’s complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: Longitudinal deviation will be positive if the aircraft is ahead of its planned position. Longitudinal deviation will be negative if the aircraft is behind its planned position.

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-074-001: Parse valid Longitudinal Deviation
- TC-4-074-002: Verify format compliance
- TC-4-074-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-075: Parse Transversal Distance Deviation (I4/075)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/075 (Transversal Distance Deviation) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Transversal distance deviation for Route Adherence Monitoring, in two’s complement form.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: Deviation to the right of the track will be coded as a positive value. Deviation to the left of the track will be coded as a negative value

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-075-001: Parse valid Transversal Distance Deviation
- TC-4-075-002: Verify format compliance
- TC-4-075-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-076: Parse Vertical Deviation (I4/076)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/076 (Vertical Deviation) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Vertical Deviation from planned altitude, in two’s complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: Positive value if aircraft is above planned altitude. Negative value if aircraft is below planned altitude.

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-076-001: Parse valid Vertical Deviation
- TC-4-076-002: Verify format compliance
- TC-4-076-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-100: Parse Area Definition (I4/100)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/100 (Area Definition) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Definition of Areas involved in a Safety Net Alert

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-100-001: Parse valid Area Definition
- TC-4-100-002: Verify format compliance
- TC-4-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-110: Parse SFDPS Sector Control Identification (I4/110)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/110 (SFDPS Sector Control Identification) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Identification of a list of FDPS Sector Control Positions in charge of the involved targets, as provided by the FDPS.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: The Centre identification code and the Control position identification code must be defined between the communication partners.

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-110-001: Parse valid SFDPS Sector Control Identification
- TC-4-110-002: Verify format compliance
- TC-4-110-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-120: Parse Conflict Characteristics (I4/120)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/120 (Conflict Characteristics) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Description of the Conflict Properties 

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-120-001: Parse valid Conflict Characteristics
- TC-4-120-002: Verify format compliance
- TC-4-120-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-170: Parse Aircraft Identification and Characteristics 1 (I4/170)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/170 (Aircraft Identification and Characteristics 1) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Identification and Characteristics of Aircraft 1 Involved in the Conflict.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-170-001: Parse valid Aircraft Identification and Characteristics 1
- TC-4-170-002: Verify format compliance
- TC-4-170-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-171: Parse Aircraft Identification and Characteristics 2 (I4/171)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/171 (Aircraft Identification and Characteristics 2) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Identification and Characteristics of Aircraft 2 Involved in the Conflict.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-171-001: Parse valid Aircraft Identification and Characteristics 2
- TC-4-171-002: Verify format compliance
- TC-4-171-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-SP: Parse SP Field (I4/SP)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/SP (SP Field) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Sp Field

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-SP-001: Parse valid SP Field
- TC-4-SP-002: Verify format compliance
- TC-4-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-4-RE: Parse Reserved Expansion Data Field (I4/RE)

**Parent:** REQ-HLR-CAT-4
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I4/RE (Reserved Expansion Data Field) as specified in the EUROCONTROL ASTERIX CAT4 v1.12 specification.

**Data Item Definition:**
Field used to introduce intermediate changes for elementary surveillance

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-RE-001: Parse valid Reserved Expansion Data Field
- TC-4-RE-002: Verify format compliance
- TC-4-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat004_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-4-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT4 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-ERR-001: Truncated data items
- TC-4-ERR-002: Invalid repetition counts
- TC-4-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-4-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT4 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-4-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-4-000 | I4/000 | Message Type | Critical | High |
| REQ-LLR-4-010 | I4/010 | Data Source Identifier | Critical | High |
| REQ-LLR-4-015 | I4/015 | SDPS Identifier | Medium | Low |
| REQ-LLR-4-020 | I4/020 | Time of Message | Critical | High |
| REQ-LLR-4-030 | I4/030 | Track Number 1 | Medium | Low |
| REQ-LLR-4-035 | I4/035 | Track Number 2 | Medium | Low |
| REQ-LLR-4-040 | I4/040 | Alert Identifier | Medium | Low |
| REQ-LLR-4-045 | I4/045 | Alert Status | Medium | Low |
| REQ-LLR-4-060 | I4/060 | Safety Net Function and System Status | Medium | Low |
| REQ-LLR-4-070 | I4/070 | Conflict Timing and Separation | Medium | Low |
| REQ-LLR-4-074 | I4/074 | Longitudinal Deviation | Medium | Low |
| REQ-LLR-4-075 | I4/075 | Transversal Distance Deviation | Medium | Low |
| REQ-LLR-4-076 | I4/076 | Vertical Deviation | Medium | Low |
| REQ-LLR-4-100 | I4/100 | Area Definition | Medium | Low |
| REQ-LLR-4-110 | I4/110 | SFDPS Sector Control Identification | Medium | Low |
| REQ-LLR-4-120 | I4/120 | Conflict Characteristics | Medium | Low |
| REQ-LLR-4-170 | I4/170 | Aircraft Identification and Characterist | Medium | Low |
| REQ-LLR-4-171 | I4/171 | Aircraft Identification and Characterist | Medium | Low |
| REQ-LLR-4-SP | I4/SP | SP Field | Medium | Low |
| REQ-LLR-4-RE | I4/RE | Reserved Expansion Data Field | Medium | Low |
| REQ-LLR-4-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-4-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 22

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-4
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-4-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT4 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
