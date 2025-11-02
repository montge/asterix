# Low-Level Requirements - ASTERIX Category 252
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT252-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-252
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 252 (Connection and Service Messages) v7.0.

**Parent Requirement:** REQ-HLR-CAT-252 - Parse ASTERIX Category 252

Each data item in CAT252 has corresponding LLRs defining parsing behavior.

---

## 2. CAT252 Data Item Requirements

### REQ-LLR-252-010: Parse Server Identification Tag (I252/010)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/010 (Server Identification Tag) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Identification of the Server of track information.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-010-001: Parse valid Server Identification Tag
- TC-252-010-002: Verify format compliance
- TC-252-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-015: Parse User Number (I252/015)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/015 (User Number) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Identification of the User of track/sensor information

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The User numbers are predefined in the User registration data base of the ARTAS Unit to which the User wants to connect.

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-015-001: Parse valid User Number
- TC-252-015-002: Verify format compliance
- TC-252-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-020: Parse Time Of Message (I252/020)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/020 (Time Of Message) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Absolute time stamping of the message in the form of elapsed time since last midnight.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: This time is given at an application level (e.g. time at which a message is filled, or time at which a served track is extrapolated), and not at the communication level (i.e. not the time at which the data-block containing the tracks is sent). The time of the day value is reset to 0 at every midnight.

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-020-001: Parse valid Time Of Message
- TC-252-020-002: Verify format compliance
- TC-252-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-035: Parse Type Of Message (I252/035)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/035 (Type Of Message) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
This data item allows for a more convenient handling of the message at the receiver side by further defining the type of transaction.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: The composition of the messages are described by the UAP attached to each FAMILY

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-035-001: Parse valid Type Of Message
- TC-252-035-002: Verify format compliance
- TC-252-035-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-040: Parse Access Key (I252/040)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/040 (Access Key) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
User password to be sent in the Connection Request.

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-040-001: Parse valid Access Key
- TC-252-040-002: Verify format compliance
- TC-252-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-045: Parse Role and Version (I252/045)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/045 (Role and Version) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Identification of the ROLE of a system and of the VERSION of interface.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-045-001: Parse valid Role and Version
- TC-252-045-002: Verify format compliance
- TC-252-045-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-050: Parse Default connection options (I252/050)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/050 (Default connection options) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
This item is used to indicate that the default values of parameters required to set a connection are selected.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-050-001: Parse valid Default connection options
- TC-252-050-002: Verify format compliance
- TC-252-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-060: Parse Geographical Area (I252/060)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/060 (Geographical Area) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Definition of geographical area in the form of a sequence of geographical points.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: Latitude and Longitude accuracy = 1 second @ 30 m.

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-060-001: Parse valid Geographical Area
- TC-252-060-002: Verify format compliance
- TC-252-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-070: Parse Lower Limit (I252/070)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/070 (Lower Limit) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Definition of the Lower limit of a Domain of Interest

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-070-001: Parse valid Lower Limit
- TC-252-070-002: Verify format compliance
- TC-252-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-080: Parse Upper Limit (I252/080)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/080 (Upper Limit) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Definition of the Upper limit of a Domain of Interest

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-080-001: Parse valid Upper Limit
- TC-252-080-002: Verify format compliance
- TC-252-080-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-090: Parse Preferred FPPS Identification tag (I252/090)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/090 (Preferred FPPS Identification tag) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Identification of the Flight Plan Data Processing System from which Flight-Plan related information shall preferably be sent to the User.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-090-001: Parse valid Preferred FPPS Identification tag
- TC-252-090-002: Verify format compliance
- TC-252-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-100: Parse Connection Related Report (I252/100)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/100 (Connection Related Report) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Report message sent by the Server to the User in relation to a connection.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-100-001: Parse valid Connection Related Report
- TC-252-100-002: Verify format compliance
- TC-252-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-110: Parse Service Identification (I252/110)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/110 (Service Identification) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Identification of a service.

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: In the case of ARTAS : only one elementary service identification bit from BS, C1 .. C5 can be set at a time.

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-110-001: Parse valid Service Identification
- TC-252-110-002: Verify format compliance
- TC-252-110-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-120: Parse Callsign selector (I252/120)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/120 (Callsign selector) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
List of aircraft Callsigns indicated as Track Selection elements in a Service Request.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-120-001: Parse valid Callsign selector
- TC-252-120-002: Verify format compliance
- TC-252-120-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-130: Parse Track Number Selector (I252/130)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/130 (Track Number Selector) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
List of Track numbers indicated as Track Selection elements in a Service Request.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: The Track Number Selector contains track numbers (see I030/040).

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-130-001: Parse valid Track Number Selector
- TC-252-130-002: Verify format compliance
- TC-252-130-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-135: Parse Aircraft Address Selector (I252/135)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/135 (Aircraft Address Selector) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
List of Mode S Aircraft Addresses indicated as Track Selection elements in a Service Request.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-135-001: Parse valid Aircraft Address Selector
- TC-252-135-002: Verify format compliance
- TC-252-135-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-137: Parse Aircraft Identification Selector (I252/137)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/137 (Aircraft Identification Selector) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
List of Aircraft Identification Mode S items indicated as Track Selection elements in a Service Request.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-137-001: Parse valid Aircraft Identification Selector
- TC-252-137-002: Verify format compliance
- TC-252-137-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-140: Parse Code Family selector (I252/140)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/140 (Code Family selector) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
List of families of Mode 3/A replies indicated as track selection elements in a Service Request.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-140-001: Parse valid Code Family selector
- TC-252-140-002: Verify format compliance
- TC-252-140-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-150: Parse Departure Airport Selector (I252/150)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/150 (Departure Airport Selector) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
List of Departure Airports indicated as Track Selection elements in a Service Request.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: The airport names are indicated in the ICAO location indicators book.

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-150-001: Parse valid Departure Airport Selector
- TC-252-150-002: Verify format compliance
- TC-252-150-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-160: Parse Destination Airport Selector (I252/160)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/160 (Destination Airport Selector) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
List of Destination Airports indicated as Track Selection elements in a Service Request.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: The airport names are indicated in the ICAO location indicators book.

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-160-001: Parse valid Destination Airport Selector
- TC-252-160-002: Verify format compliance
- TC-252-160-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-170: Parse Aircraft Type Selector (I252/170)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/170 (Aircraft Type Selector) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
List of Aircraft Types indicated as Track Selection elements in a Service Request.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: The types of aircraft are defined in ICAO Document 4444.

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-170-001: Parse valid Aircraft Type Selector
- TC-252-170-002: Verify format compliance
- TC-252-170-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-190: Parse Current Control Position Selector (I252/190)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/190 (Current Control Position Selector) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Identification of the Current Control Position to be used as track selector element in a Service Request.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The centre and the control position identification codes have to be defined between communication partners.

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-190-001: Parse valid Current Control Position Selector
- TC-252-190-002: Verify format compliance
- TC-252-190-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-200: Parse Track/Flight Categories Selector (I252/200)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/200 (Track/Flight Categories Selector) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
This item defines which categories of tracks and/or flights shall be served.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-200-001: Parse valid Track/Flight Categories Selector
- TC-252-200-002: Verify format compliance
- TC-252-200-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-210: Parse Item Selector (I252/210)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/210 (Item Selector) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
The Item Selector is used to define the composition of the track information messages. The bits that compose the Item Selector point the items composing the ARTAS Track Information UAP (ATI/UAP).

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-210-001: Parse valid Item Selector
- TC-252-210-002: Verify format compliance
- TC-252-210-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-220: Parse Cyclical Update Characteristics (I252/220)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/220 (Cyclical Update Characteristics) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Definition of the Scanning Period, Update Period, Max Data Flow and Extra Delay (to be considered for the track extrapolation) in case of cyclical service.

**Implementation Notes:**
- Format: Fixed (5 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-220-001: Parse valid Cyclical Update Characteristics
- TC-252-220-002: Verify format compliance
- TC-252-220-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-230: Parse Radar Synchronization Characteristics (I252/230)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/230 (Radar Synchronization Characteristics) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Transmission Characteristics for a service of the type radar synchronised.

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-230-001: Parse valid Radar Synchronization Characteristics
- TC-252-230-002: Verify format compliance
- TC-252-230-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-240: Parse Discrete Parameters Flags (I252/240)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/240 (Discrete Parameters Flags) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
This item contains a number of flags to indicate which category(ies) of discrete events shall be considered for triggering the track information transmission (aperiodical way of transmission).

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-240-001: Parse valid Discrete Parameters Flags
- TC-252-240-002: Verify format compliance
- TC-252-240-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-250: Parse MOF Probability Criterion (I252/250)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/250 (MOF Probability Criterion) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Variation step of Mode-of-Flight Probabilities to trigger the transmission of track information updates (i.e. an update will be sent each time the variation between the current and the last transmitted MOF probability assessment for a given track exceeds the indicated threshold value).

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-250-001: Parse valid MOF Probability Criterion
- TC-252-250-002: Verify format compliance
- TC-252-250-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-255: Parse Rate of Climb/Descent Criterion (I252/255)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/255 (Rate of Climb/Descent Criterion) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Variation step of the calculated rate of climb/descent to trigger the transmission of track information updates (i.e. an update will be sent each time the variation between the current and the last transmitted assessment of the rate of climb/ descent for a given track exceeds the indicated threshold value).

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-255-001: Parse valid Rate of Climb/Descent Criterion
- TC-252-255-002: Verify format compliance
- TC-252-255-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-260: Parse Position Criterion (I252/260)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/260 (Position Criterion) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Variation step of Track positional information to trigger the transmission of track updates. An update will be sent each time the variation between the current track position and the extrapolated position based on the previously sent information exceeds the indicated threshold.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-260-001: Parse valid Position Criterion
- TC-252-260-002: Verify format compliance
- TC-252-260-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-265: Parse Rate of Turn Criterion (I252/265)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/265 (Rate of Turn Criterion) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Variation step of the calculated rate of turn to trigger the transmission of track information updates (i.e. an update will be sent each time the variation between the current and the last transmitted assessment of the rate of turn for a given track exceeds the indicated threshold value).

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-265-001: Parse valid Rate of Turn Criterion
- TC-252-265-002: Verify format compliance
- TC-252-265-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-270: Parse Heading Criterion (I252/270)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/270 (Heading Criterion) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Variation step of heading (i.e. Speed Vector) to trigger the transmission of track information updates (i.e. an update will be sent each time the variation between the current and the last transmitted Track Heading assessment for a given track exceeds the indicated threshold value).

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-270-001: Parse valid Heading Criterion
- TC-252-270-002: Verify format compliance
- TC-252-270-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-280: Parse Speed Module Criterion (I252/280)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/280 (Speed Module Criterion) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Variation step of Speed Module to trigger the transmission of track information updates (i.e. an update will be sent each time the variation between the current and the last transmitted Speed Module assessment for a given track exceed the indicated threshold value).

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-280-001: Parse valid Speed Module Criterion
- TC-252-280-002: Verify format compliance
- TC-252-280-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-290: Parse Altitude Criterion (I252/290)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/290 (Altitude Criterion) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
A threshold for triggering the transmission of a track. A track update will be sent each time the calculated track altitude or calculated track flight level has changed by more than the indicated amount since the last track update.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-290-001: Parse valid Altitude Criterion
- TC-252-290-002: Verify format compliance
- TC-252-290-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-300: Parse Factor K (I252/300)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/300 (Factor K) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
The so-called factor K is used to trigger the data transmission as a function of the internal accuracy by which the ARTAS Tracker has estimated the state vector elements. An update will be sent each time one of the state vector elements has varied from more than K time the standard deviation associated to this element.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-300-001: Parse valid Factor K
- TC-252-300-002: Verify format compliance
- TC-252-300-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-310: Parse Refreshment Period (I252/310)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/310 (Refreshment Period) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
In relation to a service applying the aperiodical mode of transmission, the Refreshment Period concerns the systematic transmission of track updates for those tracks for which no changes have been detected.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-310-001: Parse valid Refreshment Period
- TC-252-310-002: Verify format compliance
- TC-252-310-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-320: Parse Minimal Period (I252/320)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/320 (Minimal Period) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
In relation to a service applying the aperiodical mode of transmission, a Minimal Period shall be specified to avoid that too close transmission intervals are applied when highly manoeuvring aircraft are tracked.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-320-001: Parse valid Minimal Period
- TC-252-320-002: Verify format compliance
- TC-252-320-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-330: Parse Service Related Report (I252/330)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/330 (Service Related Report) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Report message sent by the Server to the User in relation to a connection or a service.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-330-001: Parse valid Service Related Report
- TC-252-330-002: Verify format compliance
- TC-252-330-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-340: Parse Scaling Factor (I252/340)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/340 (Scaling Factor) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
This item is used to select the precision by which will be coded a cartesian track position

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-340-001: Parse valid Scaling Factor
- TC-252-340-002: Verify format compliance
- TC-252-340-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-350: Parse Sensor Selector (I252/350)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/350 (Sensor Selector) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
List of Sensor SIC/SAC identifications for which Sensor information are to be provided

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-350-001: Parse valid Sensor Selector
- TC-252-350-002: Verify format compliance
- TC-252-350-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-360: Parse Sensor Item Selector (I252/360)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/360 (Sensor Item Selector) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
This item is used to define the composition of the Sensor Information messages

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-360-001: Parse valid Sensor Item Selector
- TC-252-360-002: Verify format compliance
- TC-252-360-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-370: Parse Periodical Characteristics of Sensor Information Service (I252/370)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/370 (Periodical Characteristics of Sensor Information Service) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Update period of a periodical service of Sensor Information messages

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-370-001: Parse valid Periodical Characteristics of Sensor Information Service
- TC-252-370-002: Verify format compliance
- TC-252-370-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-400: Parse Refreshment Period of Sensor Information Service (I252/400)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/400 (Refreshment Period of Sensor Information Service) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Refreshment period of an aperiodical service of Sensor Information messages.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-400-001: Parse valid Refreshment Period of Sensor Information Service
- TC-252-400-002: Verify format compliance
- TC-252-400-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-410: Parse Discrete Parameters of Sensor Information Service (I252/410)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/410 (Discrete Parameters of Sensor Information Service) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Discrete parameters of an aperiodical service of Sensor Information messages. It defines which event(s) shall trigger a message to be sent.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-410-001: Parse valid Discrete Parameters of Sensor Information Service
- TC-252-410-002: Verify format compliance
- TC-252-410-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-420: Parse Time Stamping Criterion of Sensor Information Service (I252/420)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/420 (Time Stamping Criterion of Sensor Information Service) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Time stamping bias criterion of an aperiodical service of Sensor Information messages.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-420-001: Parse valid Time Stamping Criterion of Sensor Information Service
- TC-252-420-002: Verify format compliance
- TC-252-420-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-430: Parse SSR Range Bias Criterion of Sensor Information Service (I252/430)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/430 (SSR Range Bias Criterion of Sensor Information Service) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
SSR range bias criterion of an aperiodical service of Sensor Information messages.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-430-001: Parse valid SSR Range Bias Criterion of Sensor Information Service
- TC-252-430-002: Verify format compliance
- TC-252-430-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-440: Parse SSR Range Gain Criterion of Sensor Information Service (I252/440)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/440 (SSR Range Gain Criterion of Sensor Information Service) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
SSR range gain criterion of an aperiodical service of Sensor Information messages

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-440-001: Parse valid SSR Range Gain Criterion of Sensor Information Service
- TC-252-440-002: Verify format compliance
- TC-252-440-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-450: Parse SSR Azimuth Bias Criterion of Sensor Information Service (I252/450)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/450 (SSR Azimuth Bias Criterion of Sensor Information Service) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
SSR azimuth bias criterion of an aperiodical service of Sensor Information messages.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-450-001: Parse valid SSR Azimuth Bias Criterion of Sensor Information Service
- TC-252-450-002: Verify format compliance
- TC-252-450-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-460: Parse PR Range Bias Criterion of Sensor Information Service (I252/460)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/460 (PR Range Bias Criterion of Sensor Information Service) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Primary range bias criterion of an aperiodical service of Sensor Information messages.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-460-001: Parse valid PR Range Bias Criterion of Sensor Information Service
- TC-252-460-002: Verify format compliance
- TC-252-460-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-470: Parse PR Range Gain Criterion of Sensor Information Service (I252/470)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/470 (PR Range Gain Criterion of Sensor Information Service) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Primary range gain criterion of an aperiodical service of Sensor Information messages

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-470-001: Parse valid PR Range Gain Criterion of Sensor Information Service
- TC-252-470-002: Verify format compliance
- TC-252-470-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-252-480: Parse PR Azimuth Bias Criterion of Sensor Information Service (I252/480)

**Parent:** REQ-HLR-CAT-252
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I252/480 (PR Azimuth Bias Criterion of Sensor Information Service) as specified in the EUROCONTROL ASTERIX CAT252 v7.0 specification.

**Data Item Definition:**
Primary azimuth bias criterion of an aperiodical service of Sensor Information messages.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-480-001: Parse valid PR Azimuth Bias Criterion of Sensor Information Service
- TC-252-480-002: Verify format compliance
- TC-252-480-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat252_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-252-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT252 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-ERR-001: Truncated data items
- TC-252-ERR-002: Invalid repetition counts
- TC-252-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-252-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT252 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-252-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-252-010 | I252/010 | Server Identification Tag | Medium | Low |
| REQ-LLR-252-015 | I252/015 | User Number | Medium | Low |
| REQ-LLR-252-020 | I252/020 | Time Of Message | Medium | Low |
| REQ-LLR-252-035 | I252/035 | Type Of Message | Medium | Low |
| REQ-LLR-252-040 | I252/040 | Access Key | Medium | Low |
| REQ-LLR-252-045 | I252/045 | Role and Version | Medium | Low |
| REQ-LLR-252-050 | I252/050 | Default connection options | Medium | Low |
| REQ-LLR-252-060 | I252/060 | Geographical Area | Medium | Low |
| REQ-LLR-252-070 | I252/070 | Lower Limit | Medium | Low |
| REQ-LLR-252-080 | I252/080 | Upper Limit | Medium | Low |
| REQ-LLR-252-090 | I252/090 | Preferred FPPS Identification tag | Medium | Low |
| REQ-LLR-252-100 | I252/100 | Connection Related Report | Medium | Low |
| REQ-LLR-252-110 | I252/110 | Service Identification | Medium | Low |
| REQ-LLR-252-120 | I252/120 | Callsign selector | Medium | Low |
| REQ-LLR-252-130 | I252/130 | Track Number Selector | Medium | Low |
| REQ-LLR-252-135 | I252/135 | Aircraft Address Selector | Medium | Low |
| REQ-LLR-252-137 | I252/137 | Aircraft Identification Selector | Medium | Low |
| REQ-LLR-252-140 | I252/140 | Code Family selector | Medium | Low |
| REQ-LLR-252-150 | I252/150 | Departure Airport Selector | Medium | Low |
| REQ-LLR-252-160 | I252/160 | Destination Airport Selector | Medium | Low |
| REQ-LLR-252-170 | I252/170 | Aircraft Type Selector | Medium | Low |
| REQ-LLR-252-190 | I252/190 | Current Control Position Selector | Medium | Low |
| REQ-LLR-252-200 | I252/200 | Track/Flight Categories Selector | Medium | Low |
| REQ-LLR-252-210 | I252/210 | Item Selector | Medium | Low |
| REQ-LLR-252-220 | I252/220 | Cyclical Update Characteristics | Medium | Low |
| REQ-LLR-252-230 | I252/230 | Radar Synchronization Characteristics | Medium | Low |
| REQ-LLR-252-240 | I252/240 | Discrete Parameters Flags | Medium | Low |
| REQ-LLR-252-250 | I252/250 | MOF Probability Criterion | Medium | Low |
| REQ-LLR-252-255 | I252/255 | Rate of Climb/Descent Criterion | Medium | Low |
| REQ-LLR-252-260 | I252/260 | Position Criterion | Medium | Low |
| REQ-LLR-252-265 | I252/265 | Rate of Turn Criterion | Medium | Low |
| REQ-LLR-252-270 | I252/270 | Heading Criterion | Medium | Low |
| REQ-LLR-252-280 | I252/280 | Speed Module Criterion | Medium | Low |
| REQ-LLR-252-290 | I252/290 | Altitude Criterion | Medium | Low |
| REQ-LLR-252-300 | I252/300 | Factor K | Medium | Low |
| REQ-LLR-252-310 | I252/310 | Refreshment Period | Medium | Low |
| REQ-LLR-252-320 | I252/320 | Minimal Period | Medium | Low |
| REQ-LLR-252-330 | I252/330 | Service Related Report | Medium | Low |
| REQ-LLR-252-340 | I252/340 | Scaling Factor | Medium | Low |
| REQ-LLR-252-350 | I252/350 | Sensor Selector | Medium | Low |
| REQ-LLR-252-360 | I252/360 | Sensor Item Selector | Medium | Low |
| REQ-LLR-252-370 | I252/370 | Periodical Characteristics of Sensor Inf | Medium | Low |
| REQ-LLR-252-400 | I252/400 | Refreshment Period of Sensor Information | Medium | Low |
| REQ-LLR-252-410 | I252/410 | Discrete Parameters of Sensor Informatio | Medium | Low |
| REQ-LLR-252-420 | I252/420 | Time Stamping Criterion of Sensor Inform | Medium | Low |
| REQ-LLR-252-430 | I252/430 | SSR Range Bias Criterion of Sensor Infor | Medium | Low |
| REQ-LLR-252-440 | I252/440 | SSR Range Gain Criterion of Sensor Infor | Medium | Low |
| REQ-LLR-252-450 | I252/450 | SSR Azimuth Bias Criterion of Sensor Inf | Medium | Low |
| REQ-LLR-252-460 | I252/460 | PR Range Bias Criterion of Sensor Inform | Medium | Low |
| REQ-LLR-252-470 | I252/470 | PR Range Gain Criterion of Sensor Inform | Medium | Low |
| REQ-LLR-252-480 | I252/480 | PR Azimuth Bias Criterion of Sensor Info | Medium | Low |
| REQ-LLR-252-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-252-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 53

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-252
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-252-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT252 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
