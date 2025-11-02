# Low-Level Requirements - ASTERIX Category 62
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT62-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-62
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 62 (SDPS Track Messages) v1.18.

**Parent Requirement:** REQ-HLR-CAT-62 - Parse ASTERIX Category 62

Each data item in CAT62 has corresponding LLRs defining parsing behavior.

---

## 2. CAT62 Data Item Requirements

### REQ-LLR-62-010: Parse Data Source Identifier (I62/010)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I62/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Identification of the system sending the data

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: mandatory
- Note: The up-to-date list of SACs is published on the EUROCONTROL Web Site (http://www.eurocontrol.int/asterix).

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-010-001: Parse valid Data Source Identifier
- TC-62-010-002: Verify format compliance
- TC-62-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-62-015: Parse Service Identification (I62/015)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/015 (Service Identification) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Identification of the service provided to one or more users.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: the service identification is allocated by the system

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-015-001: Parse valid Service Identification
- TC-62-015-002: Verify format compliance
- TC-62-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-040: Parse Track Number (I62/040)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I62/040 (Track Number) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Identification of a track

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: mandatory
- Note: This Item shall be present in every ASTERIX record

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-040-001: Parse valid Track Number
- TC-62-040-002: Verify format compliance
- TC-62-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-62-060: Parse Track Mode 3/A Code (I62/060)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/060 (Track Mode 3/A Code) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Mode-3/A code converted into octal representation.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-060-001: Parse valid Track Mode 3/A Code
- TC-62-060-002: Verify format compliance
- TC-62-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-070: Parse Time Of Track Information (I62/070)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I62/070 (Time Of Track Information) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Absolute time stamping of the information provided in the track message, in the form of elapsed time since last midnight, expressed as UTC.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: mandatory
- Note: 1. This is the time of the track state vector. 2. The time is reset to zero at every midnight.

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-070-001: Parse valid Time Of Track Information
- TC-62-070-002: Verify format compliance
- TC-62-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-62-080: Parse Track Status (I62/080)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I62/080 (Track Status) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Status of a track.

**Implementation Notes:**
- Format: Variable
- Rule: mandatory
- Note: 1. Track type and coasting can also be derived from I062/290 System Track Update Ages 2. If the system supports the technology, default value (0) means that the technology was used to produce the report 3. If the system does not support the technology, default value is meaningless.

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-080-001: Parse valid Track Status
- TC-62-080-002: Verify format compliance
- TC-62-080-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-62-100: Parse Calculated Track Position. (Cartesian) (I62/100)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/100 (Calculated Track Position. (Cartesian)) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Calculated position in Cartesian co-ordinates with a resolution of 0.5m, in two's complement form.

**Implementation Notes:**
- Format: Fixed (6 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-100-001: Parse valid Calculated Track Position. (Cartesian)
- TC-62-100-002: Verify format compliance
- TC-62-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-105: Parse Calculated Position In WGS-84 Co-ordinates (I62/105)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/105 (Calculated Position In WGS-84 Co-ordinates) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Calculated Position in WGS-84 Co-ordinates with a resolution of 180/225. degrees

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: optional
- Note: The LSB provides a resolution at least better than 0.6m

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-105-001: Parse valid Calculated Position In WGS-84 Co-ordinates
- TC-62-105-002: Verify format compliance
- TC-62-105-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-110: Parse Mode 5 Data reports and Extended Mode 1 Code (I62/110)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/110 (Mode 5 Data reports and Extended Mode 1 Code) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Mode 5 Data reports and Extended Mode 1 Code

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-110-001: Parse valid Mode 5 Data reports and Extended Mode 1 Code
- TC-62-110-002: Verify format compliance
- TC-62-110-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-120: Parse Track Mode 2 Code (I62/120)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/120 (Track Mode 2 Code) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Mode 2 code associated to the track

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-120-001: Parse valid Track Mode 2 Code
- TC-62-120-002: Verify format compliance
- TC-62-120-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-130: Parse Calculated Track Geometric Altitude (I62/130)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/130 (Calculated Track Geometric Altitude) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Vertical distance between the target and the projection of its position on the earth's ellipsoid, as defined by WGS84, in two's complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 1. LSB is required to be less than 10 ft by ICAO 2. The source of altitude is identified in bits (SRC) of item I062/080 Track Status

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-130-001: Parse valid Calculated Track Geometric Altitude
- TC-62-130-002: Verify format compliance
- TC-62-130-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-135: Parse Calculated Track Barometric Altitude (I62/135)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/135 (Calculated Track Barometric Altitude) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Calculated Barometric Altitude of the track, in two's complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: ICAO specifies a range between -10 FL and 1267 FL for Mode C

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-135-001: Parse valid Calculated Track Barometric Altitude
- TC-62-135-002: Verify format compliance
- TC-62-135-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-136: Parse Measured Flight Level (I62/136)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/136 (Measured Flight Level) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Last valid and credible flight level used to update the track, in two's complement form,.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 1. The criteria to determine the credibility of the flight level are Tracker dependent. 2. Credible means: within reasonable range of change with respect to the previous detection. 3. ICAO specifies a range between -10 FL and 1267 FL for Mode C 4. This item includes the barometric altitude received from ADS-B

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-136-001: Parse valid Measured Flight Level
- TC-62-136-002: Verify format compliance
- TC-62-136-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-185: Parse Calculated Track Velocity (Cartesian) (I62/185)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/185 (Calculated Track Velocity (Cartesian)) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Calculated track velocity expressed in Cartesian co-ordinates, in two's complement form.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: The y-axis points to the Geographical North at the location of the target.

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-185-001: Parse valid Calculated Track Velocity (Cartesian)
- TC-62-185-002: Verify format compliance
- TC-62-185-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-200: Parse Mode of Movement (I62/200)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/200 (Mode of Movement) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Calculated Mode of Movement of a target.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: The ADF, if set, indicates that a difference has been detected in the altitude information derived from radar as compared to other technologies (such as ADS-B).

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-200-001: Parse valid Mode of Movement
- TC-62-200-002: Verify format compliance
- TC-62-200-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-210: Parse Calculated Acceleration (Cartesian) (I62/210)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/210 (Calculated Acceleration (Cartesian)) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Calculated Acceleration of the target expressed in Cartesian coordinates, in two's complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 1. The y-axis points to the Geographical North at the location of the target. 2. Maximum value means maximum value or above.

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-210-001: Parse valid Calculated Acceleration (Cartesian)
- TC-62-210-002: Verify format compliance
- TC-62-210-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-220: Parse Calculated Rate Of Climb/Descent) (I62/220)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/220 (Calculated Rate Of Climb/Descent)) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Calculated rate of Climb/Descent of an aircraft in two's complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: A positive value indicates a climb, whereas a negative value indicates a descent.

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-220-001: Parse valid Calculated Rate Of Climb/Descent)
- TC-62-220-002: Verify format compliance
- TC-62-220-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-245: Parse Target Identification (I62/245)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/245 (Target Identification) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Target (aircraft or vehicle) identification in 8 characters.

**Implementation Notes:**
- Format: Fixed (7 bytes)
- Rule: optional
- Note: 1. For coding, see section 3.8.2.9 of [Ref.3] 2. As the Callsign of the target can already be transmitted (thanks to I062/380 Subfield #25 if downlinked from the aircraft or thanks to I062/390 Subfield #2 if the target is correlated to a flight plan), and in order to avoid confusion at end user's side, this item SHALL not be used.

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-245-001: Parse valid Target Identification
- TC-62-245-002: Verify format compliance
- TC-62-245-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-270: Parse Target Size and Orientation (I62/270)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/270 (Target Size and Orientation) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Target size defined as length and width of the detected target, and orientation.

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 1. The orientation gives the direction which the target nose is pointing to, relative to the Geographical North. 2. When the length only is sent, the largest dimension is provided.

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-270-001: Parse valid Target Size and Orientation
- TC-62-270-002: Verify format compliance
- TC-62-270-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-290: Parse System Track Update Ages (I62/290)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/290 (System Track Update Ages) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Ages of the last plot/local track/target report update for eachsensor type.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-290-001: Parse valid System Track Update Ages
- TC-62-290-002: Verify format compliance
- TC-62-290-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-295: Parse Track Data Ages (I62/295)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/295 (Track Data Ages) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Ages of the data provided.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-295-001: Parse valid Track Data Ages
- TC-62-295-002: Verify format compliance
- TC-62-295-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-300: Parse Vehicle Fleet Identification (I62/300)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/300 (Vehicle Fleet Identification) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Vehicle fleet identification number.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-300-001: Parse valid Vehicle Fleet Identification
- TC-62-300-002: Verify format compliance
- TC-62-300-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-340: Parse Measured In Data Item Formation (I62/340)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/340 (Measured In Data Item Formation) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
All measured data related to the last report used to update the track

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-340-001: Parse valid Measured In Data Item Formation
- TC-62-340-002: Verify format compliance
- TC-62-340-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-380: Parse Aircraft Derived Data (I62/380)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/380 (Aircraft Derived Data) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Data derived directly by the aircraft

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-380-001: Parse valid Aircraft Derived Data
- TC-62-380-002: Verify format compliance
- TC-62-380-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-390: Parse Flight Plan Related Data (I62/390)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/390 (Flight Plan Related Data) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
All flight plan related information

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-390-001: Parse valid Flight Plan Related Data
- TC-62-390-002: Verify format compliance
- TC-62-390-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-500: Parse Estimated Accuracies (I62/500)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/500 (Estimated Accuracies) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Overview of all important accuracies

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-500-001: Parse valid Estimated Accuracies
- TC-62-500-002: Verify format compliance
- TC-62-500-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-510: Parse Composed Track Number (I62/510)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/510 (Composed Track Number) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Identification of a system track

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-510-001: Parse valid Composed Track Number
- TC-62-510-002: Verify format compliance
- TC-62-510-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-RE: Parse Reserved Expansion Field (I62/RE)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/RE (Reserved Expansion Field) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Reserved Expansion Field

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-RE-001: Parse valid Reserved Expansion Field
- TC-62-RE-002: Verify format compliance
- TC-62-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-62-SP: Parse Special Purpose Field (I62/SP)

**Parent:** REQ-HLR-CAT-62
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I62/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT62 v1.18 specification.

**Data Item Definition:**
Special Purpose Field

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-SP-001: Parse valid Special Purpose Field
- TC-62-SP-002: Verify format compliance
- TC-62-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-62-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT62 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-ERR-001: Truncated data items
- TC-62-ERR-002: Invalid repetition counts
- TC-62-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-62-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT62 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-62-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-62-010 | I62/010 | Data Source Identifier | Critical | High |
| REQ-LLR-62-015 | I62/015 | Service Identification | Medium | Low |
| REQ-LLR-62-040 | I62/040 | Track Number | Critical | High |
| REQ-LLR-62-060 | I62/060 | Track Mode 3/A Code | Medium | Low |
| REQ-LLR-62-070 | I62/070 | Time Of Track Information | Critical | High |
| REQ-LLR-62-080 | I62/080 | Track Status | Critical | High |
| REQ-LLR-62-100 | I62/100 | Calculated Track Position. (Cartesian) | Medium | Low |
| REQ-LLR-62-105 | I62/105 | Calculated Position In WGS-84 Co-ordinat | Medium | Low |
| REQ-LLR-62-110 | I62/110 | Mode 5 Data reports and Extended Mode 1  | Medium | Low |
| REQ-LLR-62-120 | I62/120 | Track Mode 2 Code | Medium | Low |
| REQ-LLR-62-130 | I62/130 | Calculated Track Geometric Altitude | Medium | Low |
| REQ-LLR-62-135 | I62/135 | Calculated Track Barometric Altitude | Medium | Low |
| REQ-LLR-62-136 | I62/136 | Measured Flight Level | Medium | Low |
| REQ-LLR-62-185 | I62/185 | Calculated Track Velocity (Cartesian) | Medium | Low |
| REQ-LLR-62-200 | I62/200 | Mode of Movement | Medium | Low |
| REQ-LLR-62-210 | I62/210 | Calculated Acceleration (Cartesian) | Medium | Low |
| REQ-LLR-62-220 | I62/220 | Calculated Rate Of Climb/Descent) | Medium | Low |
| REQ-LLR-62-245 | I62/245 | Target Identification | Medium | Low |
| REQ-LLR-62-270 | I62/270 | Target Size and Orientation | Medium | Low |
| REQ-LLR-62-290 | I62/290 | System Track Update Ages | Medium | Low |
| REQ-LLR-62-295 | I62/295 | Track Data Ages | Medium | Low |
| REQ-LLR-62-300 | I62/300 | Vehicle Fleet Identification | Medium | Low |
| REQ-LLR-62-340 | I62/340 | Measured In Data Item Formation | Medium | Low |
| REQ-LLR-62-380 | I62/380 | Aircraft Derived Data | Medium | Low |
| REQ-LLR-62-390 | I62/390 | Flight Plan Related Data | Medium | Low |
| REQ-LLR-62-500 | I62/500 | Estimated Accuracies | Medium | Low |
| REQ-LLR-62-510 | I62/510 | Composed Track Number | Medium | Low |
| REQ-LLR-62-RE | I62/RE | Reserved Expansion Field | Medium | Low |
| REQ-LLR-62-SP | I62/SP | Special Purpose Field | Medium | Low |
| REQ-LLR-62-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-62-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 31

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-62
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-62-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT62 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
