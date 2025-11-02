# Low-Level Requirements - ASTERIX Category 32
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT32-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-32
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 32 (Messages Provided by Users to ARTAS) v6.2.

**Parent Requirement:** REQ-HLR-CAT-32 - Parse ASTERIX Category 32

Each data item in CAT32 has corresponding LLRs defining parsing behavior.

---

## 2. CAT32 Data Item Requirements

### REQ-LLR-32-010: Parse Server Identification Tag (I32/010)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/010 (Server Identification Tag) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Identification of the Server of track information.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: In the case of miniplan exchange between ARTAS units, the Server Identification Tag corresponds to the sender of the miniplan.

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-010-001: Parse valid Server Identification Tag
- TC-32-010-002: Verify format compliance
- TC-32-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-015: Parse User Number (I32/015)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/015 (User Number) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Identification of the User of track information

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The User numbers are predefined in the User registration data base of the ARTAS Unit to which the User wants to connect. In the case of miniplan exchange between ARTAS units, the User Number corresponds to the receiver of the miniplan.

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-015-001: Parse valid User Number
- TC-32-015-002: Verify format compliance
- TC-32-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-018: Parse Data Source Identification Tag (I32/018)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/018 (Data Source Identification Tag) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Identification of the data source (system) from which the information contained in the message was originated.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: This item will be used if the system sending the minipln information (i.e. the system which is identified in I032/015 : USER NUMBER) is another system than the FPPS from which the minipln information has been received. This will be the case of e.g. an ARTAS unit forwarding minipln information to an adjacent unit.

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-018-001: Parse valid Data Source Identification Tag
- TC-32-018-002: Verify format compliance
- TC-32-018-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-020: Parse Time Of Message (I32/020)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/020 (Time Of Message) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Absolute time stamping of the message in the form of elapsed time since last midnight.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: This time is given at an application level (e.g. time at which a message is filled, or time at which a served track is extrapolated), and not at the communication level (i.e. not the time at which the data-block containing the tracks is sent). The time of the day value is reset to 0 at every midnight.

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-020-001: Parse valid Time Of Message
- TC-32-020-002: Verify format compliance
- TC-32-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-035: Parse Type Of Message (I32/035)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/035 (Type Of Message) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
This data item allows for a more convenient handling of the message at the receiver side by further defining the type of transaction.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: The composition of the messages are described by the UAP attached to each FAMILY.

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-035-001: Parse valid Type Of Message
- TC-32-035-002: Verify format compliance
- TC-32-035-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-040: Parse Track Number (I32/040)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/040 (Track Number) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Identification of an ARTAS track (track number)

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: Unlike the ARTAS track number I032/050, the track number does not change when a track enters or leaves an ARTAS Domain of Cooperation. It remains unchanged as long as the track exists.

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-040-001: Parse valid Track Number
- TC-32-040-002: Verify format compliance
- TC-32-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-050: Parse Artas Track Number (I32/050)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/050 (Artas Track Number) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Identification of an ARTAS track

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: The ARTAS UNIT IDENTIFICATION NUMBERS will be given as soon as the systems will be implemented.

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-050-001: Parse valid Artas Track Number
- TC-32-050-002: Verify format compliance
- TC-32-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-060: Parse Track Mode 3/A (I32/060)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/060 (Track Mode 3/A) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Mode 3/A identity associated to the track

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-060-001: Parse valid Track Mode 3/A
- TC-32-060-002: Verify format compliance
- TC-32-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-400: Parse Callsign (I32/400)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/400 (Callsign) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Callsign (in 7 characters) of an aircraft (provided in the Minipln).

**Implementation Notes:**
- Format: Fixed (7 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-400-001: Parse valid Callsign
- TC-32-400-002: Verify format compliance
- TC-32-400-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-410: Parse PLN Number (I32/410)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/410 (PLN Number) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
The PLN NUMBER is an integer value representing a unique reference to a Flight-plan record within a particular FPPS (Information provided in the minipln).

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-410-001: Parse valid PLN Number
- TC-32-410-002: Verify format compliance
- TC-32-410-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-420: Parse Flight Category (I32/420)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/420 (Flight Category) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Flight Category (information provided in the Minipln).

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-420-001: Parse valid Flight Category
- TC-32-420-002: Verify format compliance
- TC-32-420-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-430: Parse Type Of Aircraft (I32/430)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/430 (Type Of Aircraft) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Type of Aircraft indicated in a Minipln.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: The types of aircraft are defined in the ICAO Document 4444.

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-430-001: Parse valid Type Of Aircraft
- TC-32-430-002: Verify format compliance
- TC-32-430-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-435: Parse Category of Turbulence (I32/435)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/435 (Category of Turbulence) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Category of turbulence of an aircraft (information provided in the Minipln).

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-435-001: Parse valid Category of Turbulence
- TC-32-435-002: Verify format compliance
- TC-32-435-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-440: Parse Departure Airport (I32/440)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/440 (Departure Airport) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Departure Airport indicated in a Minipln

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: The Airport Names are indicated in the ICAO Location Indicators book

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-440-001: Parse valid Departure Airport
- TC-32-440-002: Verify format compliance
- TC-32-440-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-450: Parse Destination Airport (I32/450)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/450 (Destination Airport) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Destination Airport indicated in a Minipln

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: The Airport Names are indicated in the ICAO Location Indicators book

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-450-001: Parse valid Destination Airport
- TC-32-450-002: Verify format compliance
- TC-32-450-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-460: Parse Allocated SSR codes (I32/460)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/460 (Allocated SSR codes) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
List of successive SSR codes allocated to a flight (Information provided in the minipln).

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-460-001: Parse valid Allocated SSR codes
- TC-32-460-002: Verify format compliance
- TC-32-460-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-480: Parse Current Cleared Flight Level (I32/480)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/480 (Current Cleared Flight Level) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Current Cleared Flight Level of an aircraft, provided by a FPPS.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-480-001: Parse valid Current Cleared Flight Level
- TC-32-480-002: Verify format compliance
- TC-32-480-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-490: Parse Current Control Position (I32/490)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/490 (Current Control Position) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Identification of the Current Control Position in charge of an aircraft, as provided by a FPPS

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The centre and the control position identification codes have to be defined between communication partners.

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-490-001: Parse valid Current Control Position
- TC-32-490-002: Verify format compliance
- TC-32-490-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-32-500: Parse SUPPLEMENTARY FLIGHT DATA (I32/500)

**Parent:** REQ-HLR-CAT-32
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I32/500 (SUPPLEMENTARY FLIGHT DATA) as specified in the EUROCONTROL ASTERIX CAT32 v6.2 specification.

**Data Item Definition:**
Flight related data provided by ground based systems.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-500-001: Parse valid SUPPLEMENTARY FLIGHT DATA
- TC-32-500-002: Verify format compliance
- TC-32-500-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat032_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-32-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT32 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-ERR-001: Truncated data items
- TC-32-ERR-002: Invalid repetition counts
- TC-32-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-32-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT32 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-32-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-32-010 | I32/010 | Server Identification Tag | Medium | Low |
| REQ-LLR-32-015 | I32/015 | User Number | Medium | Low |
| REQ-LLR-32-018 | I32/018 | Data Source Identification Tag | Medium | Low |
| REQ-LLR-32-020 | I32/020 | Time Of Message | Medium | Low |
| REQ-LLR-32-035 | I32/035 | Type Of Message | Medium | Low |
| REQ-LLR-32-040 | I32/040 | Track Number | Medium | Low |
| REQ-LLR-32-050 | I32/050 | Artas Track Number | Medium | Low |
| REQ-LLR-32-060 | I32/060 | Track Mode 3/A | Medium | Low |
| REQ-LLR-32-400 | I32/400 | Callsign | Medium | Low |
| REQ-LLR-32-410 | I32/410 | PLN Number | Medium | Low |
| REQ-LLR-32-420 | I32/420 | Flight Category | Medium | Low |
| REQ-LLR-32-430 | I32/430 | Type Of Aircraft | Medium | Low |
| REQ-LLR-32-435 | I32/435 | Category of Turbulence | Medium | Low |
| REQ-LLR-32-440 | I32/440 | Departure Airport | Medium | Low |
| REQ-LLR-32-450 | I32/450 | Destination Airport | Medium | Low |
| REQ-LLR-32-460 | I32/460 | Allocated SSR codes | Medium | Low |
| REQ-LLR-32-480 | I32/480 | Current Cleared Flight Level | Medium | Low |
| REQ-LLR-32-490 | I32/490 | Current Control Position | Medium | Low |
| REQ-LLR-32-500 | I32/500 | SUPPLEMENTARY FLIGHT DATA | Medium | Low |
| REQ-LLR-32-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-32-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 21

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-32
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-32-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT32 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
