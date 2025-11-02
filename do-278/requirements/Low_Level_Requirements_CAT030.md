# Low-Level Requirements - ASTERIX Category 030
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT030-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-030
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 030 (Exchange of Air Situation Pictures) v6.2.

**Parent Requirement:** REQ-HLR-CAT-030 - Parse ASTERIX Category 030

Each data item in CAT030 has corresponding LLRs defining parsing behavior.

---

## 2. CAT030 Data Item Requirements

### REQ-LLR-030-010: Parse Server Identification Tag (I030/010)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/010 (Server Identification Tag) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Identification of the Server of track information.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-010-001: Parse valid Server Identification Tag
- TC-030-010-002: Verify format compliance
- TC-030-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-015: Parse User Number (I030/015)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/015 (User Number) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Identification of the User of track information

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The User numbers are predefined in the User registration data base of the ARTAS Unit to which the User wants to connect.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-015-001: Parse valid User Number
- TC-030-015-002: Verify format compliance
- TC-030-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-020: Parse Time Of Message (I030/020)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/020 (Time Of Message) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Absolute time stamping of the message in the form of elapsed time since last midnight.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: This is the time at which a message is filled and not the time at which the data-block containing the tracks is sent. The time of the day value is reset to 0 at every midnight.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-020-001: Parse valid Time Of Message
- TC-030-020-002: Verify format compliance
- TC-030-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-030: Parse Service Identification (I030/030)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/030 (Service Identification) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Identification of the service(s) to which a track message belongs.

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-030-001: Parse valid Service Identification
- TC-030-030-002: Verify format compliance
- TC-030-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-035: Parse Type Of Message (I030/035)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/035 (Type Of Message) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
This data item allows for a more convenient handling of the message at the receiver side by further defining the type of transaction.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: the Slave Track Promotion Messages are related to the inter-ARTAS cooperation and cannot be served to the normal track information Users.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-035-001: Parse valid Type Of Message
- TC-030-035-002: Verify format compliance
- TC-030-035-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-040: Parse Track Number (I030/040)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/040 (Track Number) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Identification of an ARTAS track (track number)

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: Unlike the ARTAS track number I030/050, the track number does not change when a track enters or leaves an ARTAS Domain of Cooperation. It remains unchanged as long as the track exists.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-040-001: Parse valid Track Number
- TC-030-040-002: Verify format compliance
- TC-030-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-050: Parse Artas Track Number (I030/050)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/050 (Artas Track Number) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Identification of an ARTAS track

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: The ARTAS UNIT IDENTIFICATION NUMBERS will be given as soon as the systems will be implemented.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-050-001: Parse valid Artas Track Number
- TC-030-050-002: Verify format compliance
- TC-030-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-060: Parse Track Mode 3/A (I030/060)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/060 (Track Mode 3/A) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Mode 3/A identity associated to the track

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: A change of track Mode 3/A (C = 1) is indicated during 30 seconds after the code has changed. (V) and (G) are extracted from the last Mode 3/A that was used to update the track.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-060-001: Parse valid Track Mode 3/A
- TC-030-060-002: Verify format compliance
- TC-030-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-070: Parse Time Of Last Update (I030/070)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/070 (Time Of Last Update) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Absolute time stamping of the information provided in the track message, in the form of elapsed time since last midnight.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: In ARTAS, this is the time at which the track is extrapolated by the Server in case of periodical service. Otherwise, this is the update time of the track state vector by the Tracker. The time of the day value is reset to 0 at every midnight.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-070-001: Parse valid Time Of Last Update
- TC-030-070-002: Verify format compliance
- TC-030-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-080: Parse ARTAS Track Status (I030/080)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/080 (ARTAS Track Status) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Status of an ARTAS track.

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-080-001: Parse valid ARTAS Track Status
- TC-030-080-002: Verify format compliance
- TC-030-080-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-090: Parse ARTAS Track Quality (I030/090)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/090 (ARTAS Track Quality) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
ARTAS Track Quality

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-090-001: Parse valid ARTAS Track Quality
- TC-030-090-002: Verify format compliance
- TC-030-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-100: Parse Calculated Track Position (I030/100)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/100 (Calculated Track Position) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Calculated position of an aircraft expressed in Cartesian coordinates.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: A scaling factor of f=1 is required for any User defining a Domain of Interest extending beyond a square of approximately 1024 NM X 1024 NM centred on the Unit Centre Point (i.e. the origin of the stereographic projection) of the ARTAS Unit to which it is connected. In this case, LSB will be 1/32 NM and the maximum coding range will be approximately 2048 NM (- 1024 NM .. 1023.968 NM ). The scaling factor is defined in item I252/340.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-100-001: Parse valid Calculated Track Position
- TC-030-100-002: Verify format compliance
- TC-030-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-110: Parse Estimated Accuracy of Track Position (Cartesian) (I030/110)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/110 (Estimated Accuracy of Track Position (Cartesian)) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Estimated accuracy (i.e. standard deviation) of the calculated position of an aircraft expressed in Cartesian coordinates.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: Accuracy concerns both X and Y coordinate components.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-110-001: Parse valid Estimated Accuracy of Track Position (Cartesian)
- TC-030-110-002: Verify format compliance
- TC-030-110-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-120: Parse Track Mode 2 Code (I030/120)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/120 (Track Mode 2 Code) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Mode 2 code associated to the track

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: A change of track Mode 2 (C = 1) is indicated during 30 seconds after the code has changed. (V) and (G) are extracted from the last Mode 2 that was used to update the track.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-120-001: Parse valid Track Mode 2 Code
- TC-030-120-002: Verify format compliance
- TC-030-120-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-130: Parse Calculated Track Altitude (I030/130)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/130 (Calculated Track Altitude) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Calculated altitude of an aircraft.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: SRC indicates the source used to compute the altitude.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-130-001: Parse valid Calculated Track Altitude
- TC-030-130-002: Verify format compliance
- TC-030-130-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-135: Parse Estimated Accuracy of Track Altitude (I030/135)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/135 (Estimated Accuracy of Track Altitude) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Estimated accuracy (i.e. standard deviation) of the calculated altitude of an aircraft.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-135-001: Parse valid Estimated Accuracy of Track Altitude
- TC-030-135-002: Verify format compliance
- TC-030-135-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-140: Parse Last Measured Mode C (I030/140)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/140 (Last Measured Mode C) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Mode C code of the last nearest neighbour plot containing a Mode C and used to update the track.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The VAL and GC indications are derived from the local track/plot message.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-140-001: Parse valid Last Measured Mode C
- TC-030-140-002: Verify format compliance
- TC-030-140-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-150: Parse Measured Track Mode C (I030/150)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/150 (Measured Track Mode C) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Last validated and credible Mode C value used to update the track.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: There are tracker parameters that determine whether the validity and garbled indicators are considered in the determination of the last validated and credible Mode C.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-150-001: Parse valid Measured Track Mode C
- TC-030-150-002: Verify format compliance
- TC-030-150-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-160: Parse Calculated Track Flight Level (I030/160)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/160 (Calculated Track Flight Level) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Calculated Flight Level of the track (isobar).

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The QNH is not processed in ARTAS : bit QNC is always 0.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-160-001: Parse valid Calculated Track Flight Level
- TC-030-160-002: Verify format compliance
- TC-030-160-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-165: Parse Estimated Accuracy of Calculated Track Flight Level (I030/165)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/165 (Estimated Accuracy of Calculated Track Flight Level) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Estimated accuracy (i.e. standard deviation) of the calculated Flight Level of the track (isobar).

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-165-001: Parse valid Estimated Accuracy of Calculated Track Flight Level
- TC-030-165-002: Verify format compliance
- TC-030-165-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-170: Parse Track Ages (I030/170)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/170 (Track Ages) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
A set of track related ages.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: The ages are counted from I030/070 TIME OF LAST UPDATE, using the following formula : age = I030/070 - time. If the computed age is greater than the maximum value or if the associated information has never been received, then the maximum value is returned. If the computed age is negative, then 0 is returned. The track mode A is contained in I030/060 TRACK MODE 3/A. The last valid and credible mode C is contained in I030/150 MEASURED TRACK MODE C.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-170-001: Parse valid Track Ages
- TC-030-170-002: Verify format compliance
- TC-030-170-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-180: Parse Calculated Track Velocity (Polar) (I030/180)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/180 (Calculated Track Velocity (Polar)) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Calculated track velocity expressed in polar coordinates.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-180-001: Parse valid Calculated Track Velocity (Polar)
- TC-030-180-002: Verify format compliance
- TC-030-180-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-181: Parse Calculated Track Velocity (Cartesian) (I030/181)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/181 (Calculated Track Velocity (Cartesian)) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Calculated track velocity expressed in Cartesian coordinates.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-181-001: Parse valid Calculated Track Velocity (Cartesian)
- TC-030-181-002: Verify format compliance
- TC-030-181-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-190: Parse Estimated Accuracy of Track Velocity (Polar) (I030/190)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/190 (Estimated Accuracy of Track Velocity (Polar)) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Estimated accuracy (i.e. standard deviation) of the calculated track velocity expressed in polar coordinates.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-190-001: Parse valid Estimated Accuracy of Track Velocity (Polar)
- TC-030-190-002: Verify format compliance
- TC-030-190-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-191: Parse Estimated Accuracy of Track Velocity (Cartesian) (I030/191)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/191 (Estimated Accuracy of Track Velocity (Cartesian)) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Estimated accuracy (i.e. standard deviation) of the calculated track velocity expressed in Cartesian coordinates.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-191-001: Parse valid Estimated Accuracy of Track Velocity (Cartesian)
- TC-030-191-002: Verify format compliance
- TC-030-191-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-200: Parse Mode of Flight (I030/200)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/200 (Mode of Flight) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Calculated Mode-of-Flight of an aircraft.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-200-001: Parse valid Mode of Flight
- TC-030-200-002: Verify format compliance
- TC-030-200-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-210: Parse Mode of Flight Probabilities (I030/210)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/210 (Mode of Flight Probabilities) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Probabilities attached to the Transversal, longitudinal and Vertical components of a calculated Mode-of-Flight.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-210-001: Parse valid Mode of Flight Probabilities
- TC-030-210-002: Verify format compliance
- TC-030-210-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-220: Parse Calculated Rate Of Climb/Descent (I030/220)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/220 (Calculated Rate Of Climb/Descent) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Calculated rate of Climb/Descent of an aircraft

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: A positive value represents a rate of climb and a negative value represents a rate of descent.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-220-001: Parse valid Calculated Rate Of Climb/Descent
- TC-030-220-002: Verify format compliance
- TC-030-220-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-230: Parse Estimated Accuracy of Rate of Climb/Descend (I030/230)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/230 (Estimated Accuracy of Rate of Climb/Descend) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Estimated accuracy (i.e. standard deviation) of the calculated rate of Climb/Descent of an aircraft.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-230-001: Parse valid Estimated Accuracy of Rate of Climb/Descend
- TC-030-230-002: Verify format compliance
- TC-030-230-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-240: Parse Calculated Rate Of Turn (I030/240)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/240 (Calculated Rate Of Turn) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Calculated Rate of Turn expressed in degrees per second.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: A positive value represents a right turn, whereas a negative value represents a left turn.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-240-001: Parse valid Calculated Rate Of Turn
- TC-030-240-002: Verify format compliance
- TC-030-240-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-250: Parse Estimated Accuracy of Rate of Turn (I030/250)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/250 (Estimated Accuracy of Rate of Turn) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Estimated accuracy (i.e. standard deviation) of a calculated Rate of Turn expressed in degrees per second.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-250-001: Parse valid Estimated Accuracy of Rate of Turn
- TC-030-250-002: Verify format compliance
- TC-030-250-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-260: Parse Radar Identification Tag (I030/260)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/260 (Radar Identification Tag) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Identification of the radar station from which has been received the last plot/local track used to update an ARTAS track.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-260-001: Parse valid Radar Identification Tag
- TC-030-260-002: Verify format compliance
- TC-030-260-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-270: Parse Local Track Number (I030/270)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/270 (Local Track Number) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
The local track number is an integer value representing a unique reference to a track record within a particular track file e.g. the track data-base of a radar local tracker.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-270-001: Parse valid Local Track Number
- TC-030-270-002: Verify format compliance
- TC-030-270-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-290: Parse Plot Ages (I030/290)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/290 (Plot Ages) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
A set of plot related ages.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The ages are counted from I030/070 TIME OF LAST UPDATE, using the following formula : age = I030/070 - time. If the computed age is greater than the maximum value or if the associated information has never been received, then the maximum value is returned. If the computed age is negative, then 0 is returned.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-290-001: Parse valid Plot Ages
- TC-030-290-002: Verify format compliance
- TC-030-290-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-340: Parse Last Measured Mode 3/A (I030/340)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/340 (Last Measured Mode 3/A) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Mode 3/A of the last nearest neighbour plot containing a mode 3/A and used to update the track.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: Smoothed MODE 3/A data (L = 1) will be used in case of absence of MODE 3/A code information in the plot or in case of difference between plot and local track MODE 3/A code information.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-340-001: Parse valid Last Measured Mode 3/A
- TC-030-340-002: Verify format compliance
- TC-030-340-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-360: Parse Measured Position (I030/360)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/360 (Measured Position) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Measured position of an aircraft in local polar coordinates (information provided in the last plot/local track used to update an ARTAS track).

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: The measured position is : a. In case of a plot, the measured raw polar coordinates, b. In case of a local track, the measured raw polar coordinates of the plot associated to the track, c. In case of a local track without detection, the extrapolated polar coordinates.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-360-001: Parse valid Measured Position
- TC-030-360-002: Verify format compliance
- TC-030-360-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-370: Parse Measured 3-D Height (I030/370)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/370 (Measured 3-D Height) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Height of an aircraft measured by a 3-D Radar (Information provided in the last plot/local track used to update an ARTAS track).

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-370-001: Parse valid Measured 3-D Height
- TC-030-370-002: Verify format compliance
- TC-030-370-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-382: Parse Aircraft address (I030/382)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/382 (Aircraft address) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Technical Mode-S address used for identification of an aircraft

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-382-001: Parse valid Aircraft address
- TC-030-382-002: Verify format compliance
- TC-030-382-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-384: Parse Aircraft Identification (I030/384)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/384 (Aircraft Identification) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Aircraft Identification (in 8 characters) obtained from an aircraft equipped with a Mode S transponder.

**Implementation Notes:**
- Format: Fixed (6 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-384-001: Parse valid Aircraft Identification
- TC-030-384-002: Verify format compliance
- TC-030-384-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-386: Parse Communications Capability and Flight Status (I030/386)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/386 (Communications Capability and Flight Status) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Communications capability of the transponder and flight status.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-386-001: Parse valid Communications Capability and Flight Status
- TC-030-386-002: Verify format compliance
- TC-030-386-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-390: Parse FPPS Identification tag (I030/390)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/390 (FPPS Identification tag) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Identification of the Flight Plan Data Processing System from which Flight-Plan related information are preferred by the User

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-390-001: Parse valid FPPS Identification tag
- TC-030-390-002: Verify format compliance
- TC-030-390-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-400: Parse Callsign (I030/400)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/400 (Callsign) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Callsign (in 7 characters) of an aircraft (provided in the Minipln).

**Implementation Notes:**
- Format: Fixed (7 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-400-001: Parse valid Callsign
- TC-030-400-002: Verify format compliance
- TC-030-400-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-410: Parse PLN Number (I030/410)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/410 (PLN Number) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
The PLN NUMBER is an integer value representing a unique reference to a Flight-plan record within a particular FPPS (Information provided in the minipln).

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-410-001: Parse valid PLN Number
- TC-030-410-002: Verify format compliance
- TC-030-410-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-420: Parse Flight Category (I030/420)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/420 (Flight Category) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Flight Category (information provided in the Minipln).

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-420-001: Parse valid Flight Category
- TC-030-420-002: Verify format compliance
- TC-030-420-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-430: Parse Type Of Aircraft (I030/430)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/430 (Type Of Aircraft) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Type of Aircraft indicated in a Minipln.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: The types of aircraft are defined in the ICAO Document 4444.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-430-001: Parse valid Type Of Aircraft
- TC-030-430-002: Verify format compliance
- TC-030-430-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-435: Parse Category of Turbulence (I030/435)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/435 (Category of Turbulence) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Category of turbulence of an aircraft (information provided in the Minipln).

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-435-001: Parse valid Category of Turbulence
- TC-030-435-002: Verify format compliance
- TC-030-435-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-440: Parse Departure Airport (I030/440)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/440 (Departure Airport) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Departure Airport indicated in a Minipln

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: The Airport Names are indicated in the ICAO Location Indicators book

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-440-001: Parse valid Departure Airport
- TC-030-440-002: Verify format compliance
- TC-030-440-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-450: Parse Destination Airport (I030/450)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/450 (Destination Airport) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Destination Airport indicated in a Minipln

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: The Airport Names are indicated in the ICAO Location Indicators book

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-450-001: Parse valid Destination Airport
- TC-030-450-002: Verify format compliance
- TC-030-450-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-460: Parse Allocated SSR codes (I030/460)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/460 (Allocated SSR codes) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
List of successive SSR codes allocated to a flight (Information provided in the minipln).

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-460-001: Parse valid Allocated SSR codes
- TC-030-460-002: Verify format compliance
- TC-030-460-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-480: Parse Current Cleared Flight Level (I030/480)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/480 (Current Cleared Flight Level) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Current Cleared Flight Level of an aircraft, provided by a FPPS.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-480-001: Parse valid Current Cleared Flight Level
- TC-030-480-002: Verify format compliance
- TC-030-480-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-490: Parse Current Control Position (I030/490)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/490 (Current Control Position) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Identification of the Current Control Position in charge of an aircraft, as provided by a FPPS

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The centre and the control position identification codes have to be defined between communication partners.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-490-001: Parse valid Current Control Position
- TC-030-490-002: Verify format compliance
- TC-030-490-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-030-RE: Parse Reserved Expansion Data Field (I030/RE)

**Parent:** REQ-HLR-CAT-030
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I030/RE (Reserved Expansion Data Field) as specified in the EUROCONTROL ASTERIX CAT030 v6.2 specification.

**Data Item Definition:**
Field used to introduce intermediate changes for elementary surveillance

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-RE-001: Parse valid Reserved Expansion Data Field
- TC-030-RE-002: Verify format compliance
- TC-030-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat030_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-030-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT030 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-ERR-001: Truncated data items
- TC-030-ERR-002: Invalid repetition counts
- TC-030-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-030-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT030 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-030-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-030-010 | I030/010 | Server Identification Tag | Medium | Low |
| REQ-LLR-030-015 | I030/015 | User Number | Medium | Low |
| REQ-LLR-030-020 | I030/020 | Time Of Message | Medium | Low |
| REQ-LLR-030-030 | I030/030 | Service Identification | Medium | Low |
| REQ-LLR-030-035 | I030/035 | Type Of Message | Medium | Low |
| REQ-LLR-030-040 | I030/040 | Track Number | Medium | Low |
| REQ-LLR-030-050 | I030/050 | Artas Track Number | Medium | Low |
| REQ-LLR-030-060 | I030/060 | Track Mode 3/A | Medium | Low |
| REQ-LLR-030-070 | I030/070 | Time Of Last Update | Medium | Low |
| REQ-LLR-030-080 | I030/080 | ARTAS Track Status | Medium | Low |
| REQ-LLR-030-090 | I030/090 | ARTAS Track Quality | Medium | Low |
| REQ-LLR-030-100 | I030/100 | Calculated Track Position | Medium | Low |
| REQ-LLR-030-110 | I030/110 | Estimated Accuracy of Track Position (Ca | Medium | Low |
| REQ-LLR-030-120 | I030/120 | Track Mode 2 Code | Medium | Low |
| REQ-LLR-030-130 | I030/130 | Calculated Track Altitude | Medium | Low |
| REQ-LLR-030-135 | I030/135 | Estimated Accuracy of Track Altitude | Medium | Low |
| REQ-LLR-030-140 | I030/140 | Last Measured Mode C | Medium | Low |
| REQ-LLR-030-150 | I030/150 | Measured Track Mode C | Medium | Low |
| REQ-LLR-030-160 | I030/160 | Calculated Track Flight Level | Medium | Low |
| REQ-LLR-030-165 | I030/165 | Estimated Accuracy of Calculated Track F | Medium | Low |
| REQ-LLR-030-170 | I030/170 | Track Ages | Medium | Low |
| REQ-LLR-030-180 | I030/180 | Calculated Track Velocity (Polar) | Medium | Low |
| REQ-LLR-030-181 | I030/181 | Calculated Track Velocity (Cartesian) | Medium | Low |
| REQ-LLR-030-190 | I030/190 | Estimated Accuracy of Track Velocity (Po | Medium | Low |
| REQ-LLR-030-191 | I030/191 | Estimated Accuracy of Track Velocity (Ca | Medium | Low |
| REQ-LLR-030-200 | I030/200 | Mode of Flight | Medium | Low |
| REQ-LLR-030-210 | I030/210 | Mode of Flight Probabilities | Medium | Low |
| REQ-LLR-030-220 | I030/220 | Calculated Rate Of Climb/Descent | Medium | Low |
| REQ-LLR-030-230 | I030/230 | Estimated Accuracy of Rate of Climb/Desc | Medium | Low |
| REQ-LLR-030-240 | I030/240 | Calculated Rate Of Turn | Medium | Low |
| REQ-LLR-030-250 | I030/250 | Estimated Accuracy of Rate of Turn | Medium | Low |
| REQ-LLR-030-260 | I030/260 | Radar Identification Tag | Medium | Low |
| REQ-LLR-030-270 | I030/270 | Local Track Number | Medium | Low |
| REQ-LLR-030-290 | I030/290 | Plot Ages | Medium | Low |
| REQ-LLR-030-340 | I030/340 | Last Measured Mode 3/A | Medium | Low |
| REQ-LLR-030-360 | I030/360 | Measured Position | Medium | Low |
| REQ-LLR-030-370 | I030/370 | Measured 3-D Height | Medium | Low |
| REQ-LLR-030-382 | I030/382 | Aircraft address | Medium | Low |
| REQ-LLR-030-384 | I030/384 | Aircraft Identification | Medium | Low |
| REQ-LLR-030-386 | I030/386 | Communications Capability and Flight Sta | Medium | Low |
| REQ-LLR-030-390 | I030/390 | FPPS Identification tag | Medium | Low |
| REQ-LLR-030-400 | I030/400 | Callsign | Medium | Low |
| REQ-LLR-030-410 | I030/410 | PLN Number | Medium | Low |
| REQ-LLR-030-420 | I030/420 | Flight Category | Medium | Low |
| REQ-LLR-030-430 | I030/430 | Type Of Aircraft | Medium | Low |
| REQ-LLR-030-435 | I030/435 | Category of Turbulence | Medium | Low |
| REQ-LLR-030-440 | I030/440 | Departure Airport | Medium | Low |
| REQ-LLR-030-450 | I030/450 | Destination Airport | Medium | Low |
| REQ-LLR-030-460 | I030/460 | Allocated SSR codes | Medium | Low |
| REQ-LLR-030-480 | I030/480 | Current Cleared Flight Level | Medium | Low |
| REQ-LLR-030-490 | I030/490 | Current Control Position | Medium | Low |
| REQ-LLR-030-RE | I030/RE | Reserved Expansion Data Field | Medium | Low |
| REQ-LLR-030-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-030-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 54

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-030
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-030-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT030 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
