# Low-Level Requirements - ASTERIX Category 15
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT15-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-15
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 15 (INCS Target Reports) v1.2.

**Parent Requirement:** REQ-HLR-CAT-15 - Parse ASTERIX Category 15

Each data item in CAT15 has corresponding LLRs defining parsing behavior.

---

## 2. CAT15 Data Item Requirements

### REQ-LLR-15-000: Parse Message Type (I15/000)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I15/000 (Message Type) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
This data item conveys the report type and whether the output is periodically updated or asynchronous depending upon external events.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: mandatory
- Note: 1. See Section 4.7 and ANNEX A for definitions of the Message Types. 2. Values 6 to 127 are reserved for future use 3. Periodic Report: A periodic report is one transmitted periodically with an independently configurable period. 4. Event Driven Report: An Event Driven Report is one generated in response to the occurrence of an external event such as an RF echo off a target.

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-000-001: Parse valid Message Type
- TC-15-000-002: Verify format compliance
- TC-15-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-15-010: Parse Data Source Identifier (I15/010)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I15/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Identification of the sensor from which the data is received.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: mandatory
- Note: 1. The up-to-date list of SACs is published on the EUROCONTROL Web Site 2. The SICs are allocated by the national authority responsible for the surveillance infrastructure. 3. The SIC and SAC values shall be formatted as binary unsigned integers.

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-010-001: Parse valid Data Source Identifier
- TC-15-010-002: Verify format compliance
- TC-15-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-15-015: Parse Service Identification (I15/015)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/015 (Service Identification) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Identification of the service provided to one or more users.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 1. The Service Identification is allocated by the system. 2. The SID value shall be formatted as binary unsigned integers.

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-015-001: Parse valid Service Identification
- TC-15-015-002: Verify format compliance
- TC-15-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-020: Parse Target Report Descriptor (I15/020)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I15/020 (Target Report Descriptor) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Type and characteristics of the data as transmitted by a system.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: mandatory
- Note: 1. The MoMu bit is used to indicate whether the target report was constructed from a multi-static (including bi-static) or mono-static sensor. Its setting dictates the interpretation of data items I015/625 and I015/626. The meaning of the value “other” shall be described in the system ICD. 2. In this context, a Reference Target Report stems from a non-aircraft target based on RF received externally to the system boundary. This may be generated, for example, by an external RF generator or a Permanent Echo or from a device, which is deployed in line of sight of the sensor. 3. A synthetic target is an internally generated diagnostic signal prior to the generation of the ASTERIX Category 015 target report. For example used to support test processes. 4. This value is used to represent externally generated targets or recorded data injected into the output data stream of the INCS system e.g. for test or training purposes. 5. This indication is used to inform about the scanning direction of the system (e.g. left/right, up/down, clockwise/anti-clockwise). It's exact meaning is implementation dependent and shall be described in the system ICD.

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-020-001: Parse valid Target Report Descriptor
- TC-15-020-002: Verify format compliance
- TC-15-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-15-030: Parse Warning/Error Conditions (I15/030)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/030 (Warning/Error Conditions) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Warning/error conditions detected by a system for the target report involved.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 1. It has to be stressed that a series of one or more W/E conditions can be reported per target report. 2. The nature of the warning / error condition may differ between sensor types and the declaration and use of such alerts is driven by end user requirements. 3. Potential applications could be to indicate that the target report correlates with road infrastructure (terrestrial vehicles) or a wind turbine or that it is a fixed or slow moving return or originating from an area of high clutter. Such data items could also be used to indicate the presence of interference – either deliberate or accidental. 4. The Warning/Error Condition Values from 1-31 are reserved for designation by the ASTERIX Maintenance Group. System implementers are free to use values of 32 and above. The allocation of the remaining values of this data item shall be defined in a local Interface Control Document. 5. The value of “0” must not be assigned.

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-030-001: Parse valid Warning/Error Conditions
- TC-15-030-002: Verify format compliance
- TC-15-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-050: Parse Update Period (I15/050)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/050 (Update Period) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Period until next expected output of a target report for this target.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 1. It is not necessary that all targets detected by the INCS sensor have target reports generated at the same update period. 2. This data item indicates the period until the next expected output of a target report for this target relative to the Time of Applicability contained in data item I015/145

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-050-001: Parse valid Update Period
- TC-15-050-002: Verify format compliance
- TC-15-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-145: Parse Time of Applicability (I15/145)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I15/145 (Time of Applicability) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Absolute time stamping for applicability of the measured information expressed as UTC.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: mandatory
- Note: 1. The Time of Applicability refers to the information contained in data item I015/600, I015/601, I015/625, I015/626, I015/627, I015/628 whichever is available. In case of a Track End Message (Message Type = 5) it refers to the time at which the track is terminated and the track number (data item I015/161) is released for re-use. 2. A distributed sensor, such as an MSPSR, may have multiple elements that are each individually time stamped which are consolidated in to a target report. Rather than provide details of each time stamped message, this data item conveys the time of applicability of position of the target report. 3. The Time of Applicability value is reset to zero each day at midnight. 4. The Time of Applicability value shall be formatted as a binary unsigned integer.

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-145-001: Parse valid Time of Applicability
- TC-15-145-002: Verify format compliance
- TC-15-145-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-15-161: Parse Track/Plot Number (I15/161)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I15/161 (Track/Plot Number) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
An integer value representing a unique reference to a track/plot record.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: mandatory
- Note: 1. Track numbers are required for Sensor and Measurement Tracks. However, for Sensor and Measurement Plots the inclusion of a track number is optional – depending upon whether the INCS sensor has used tracking processing to reduce the false alarm rates. 2. The track number is allocated by the system. 3. The track number value shall be formatted as binary unsigned integers.

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-161-001: Parse valid Track/Plot Number
- TC-15-161-002: Verify format compliance
- TC-15-161-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-15-170: Parse Track/Plot Status (I15/170)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I15/170 (Track/Plot Status) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Status of Track/Plot.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: mandatory
- Note: 1. The indication for CSTP and/or CSTH applies only to data items I015/600 and I015/605 respectively. In case one of these data items is not present, CSTP and/or CSTH has no meaning. 2. A coasted track is one for which the sensor detections have been interrupted and whose position/height is being predicted based on the previously received responses. 3. The blind zone or blanked zone are predictable zones where no detection is predicted. If bit 5 is set and TTS = 1 then the track is coasted because it is in a blind zone or sector blank zone. 4. The indication TUR=1 shall be sent only with Message Type = 5 "Track End Message".

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-170-001: Parse valid Track/Plot Status
- TC-15-170-002: Verify format compliance
- TC-15-170-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-15-270: Parse Target Size and Orientation (I15/270)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/270 (Target Size and Orientation) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Data item containing the size and orientation information of the target.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-270-001: Parse valid Target Size and Orientation
- TC-15-270-002: Verify format compliance
- TC-15-270-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-300: Parse Object Classification (I15/300)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/300 (Object Classification) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Classification result of the object detection.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: 1. INCS processing may be able to provide an indication of the nature of the target e.g. road vehicle or aircraft with the potential for further discrimination in the type of the aircraft e.g. two engine, fixed wing/helicopter etc. Before including requirements for target classification it is necessary to consider the operational manner in which such information would be used and how/if such data would be made available to the controllers. 2. Target classification is attributing, with an acceptable degree of confidence, a target report as having originated from a specific object or target type e.g. fixed wing aircraft, flock of birds etc. (It should be noted that the probabilities assigned to a target do not necessarily need to add up to 100%) The ability of an INCS sensor to classify the targets it detects is dependent upon the systems capabilities and is driven by end user requirements. The use of this optional data item is to be agreed between parties such as the system manufacturer, the system operator agency and the end user. The allocation of the CLS octet is to be defined in a local Interface Control Document that shall be agreed by both parties. 3. If this functionality is implemented in the sensor, the classification result (CLS), PRB and REP value shall be formatted as a binary unsigned integer.

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-300-001: Parse valid Object Classification
- TC-15-300-002: Verify format compliance
- TC-15-300-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-400: Parse Measurement Identifier (I15/400)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/400 (Measurement Identifier) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
An identifier pointing to a measurement that was created from a specific contributing Tx/Rx Pair where the Pair Identifier refers to the index which details both the transmitter characteristics (DVB-T, DAB, FM, dedicated etc.) and the receiver characteristics. These are defined in ASTERIX Category 016 – Data Item I016/300).

**Implementation Notes:**
- Format: Fixed (5 bytes)
- Rule: optional
- Note: 1. INCS sensors may achieve their operational requirements based upon different techniques and technologies. Some may utilise multiple transmitter stations or multiple receiver stations. This data items provides the means for subsequent processing stages to be able to analyse the target report data based upon the system components that contributed to the formation of the target report. See ANNEX A for further details. 2. The Pair Identifier shall be defined in ASTERIX Category 016 (Data Item I016/300). 3. The Observation Number is identifying an element of 'raw data' information. The sensor plots and sensor tracks are usually composed of several raw data information. Therefore, the Observation number is defined in the Measurement Plots and Measurement Tracks.

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-400-001: Parse valid Measurement Identifier
- TC-15-400-002: Verify format compliance
- TC-15-400-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-480: Parse Associations (I15/480)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/480 (Associations) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Information on which Measurement Identifiers contributed to the Sensor Centric Plot / Sensor Centric Track.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-480-001: Parse valid Associations
- TC-15-480-002: Verify format compliance
- TC-15-480-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-600: Parse Horizontal Position Information (I15/600)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/600 (Horizontal Position Information) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Data item containing the horizontal position information of the target.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-600-001: Parse valid Horizontal Position Information
- TC-15-600-002: Verify format compliance
- TC-15-600-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-601: Parse Geometric Height Information (I15/601)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/601 (Geometric Height Information) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Data item containing the geometric height information of the target in WGS 84 height above ellipsoid.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-601-001: Parse valid Geometric Height Information
- TC-15-601-002: Verify format compliance
- TC-15-601-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-602: Parse Horizontal Velocity Information (I15/602)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/602 (Horizontal Velocity Information) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Magnitude of the Horizontal Velocity Vector.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-602-001: Parse valid Horizontal Velocity Information
- TC-15-602-002: Verify format compliance
- TC-15-602-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-603: Parse Horizontal Acceleration Information (I15/603)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/603 (Horizontal Acceleration Information) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Magnitude of the Horizontal Acceleration Vector.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-603-001: Parse valid Horizontal Acceleration Information
- TC-15-603-002: Verify format compliance
- TC-15-603-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-604: Parse Vertical Velocity Information (I15/604)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/604 (Vertical Velocity Information) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Vertical velocity as given by the rate of change of the Geometric Height.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-604-001: Parse valid Vertical Velocity Information
- TC-15-604-002: Verify format compliance
- TC-15-604-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-605: Parse Vertical Acceleration Information (I15/605)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/605 (Vertical Acceleration Information) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Rate of change of vertical speed.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-605-001: Parse valid Vertical Acceleration Information
- TC-15-605-002: Verify format compliance
- TC-15-605-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-625: Parse Range Information (I15/625)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/625 (Range Information) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
The targets range information is given relative to the sensor reference point(s).

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-625-001: Parse valid Range Information
- TC-15-625-002: Verify format compliance
- TC-15-625-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-626: Parse Doppler Information (I15/626)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/626 (Doppler Information) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Doppler measurement of the Target.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-626-001: Parse valid Doppler Information
- TC-15-626-002: Verify format compliance
- TC-15-626-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-627: Parse Azimuth Information (I15/627)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/627 (Azimuth Information) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Azimuth information that is provided relative to the sensor or component reference point.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-627-001: Parse valid Azimuth Information
- TC-15-627-002: Verify format compliance
- TC-15-627-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-628: Parse Elevation Information (I15/628)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/628 (Elevation Information) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Information related to the elevation angle provided by the sensor. (Predominantly used by electro-optic sensors).

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-628-001: Parse valid Elevation Information
- TC-15-628-002: Verify format compliance
- TC-15-628-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-630: Parse Path Quality (I15/630)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/630 (Path Quality) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Measure characterising the signal quality associated with a specific target echo signal.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-630-001: Parse valid Path Quality
- TC-15-630-002: Verify format compliance
- TC-15-630-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-631: Parse Contour (Azimuth, Elevation Angle, Range Extent) (I15/631)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/631 (Contour (Azimuth, Elevation Angle, Range Extent)) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
Azimuth, elevation angles and range extent of all elementary presences constituting a plot.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: 1. The azimuth shall increment in a clockwise manner relative to geographic North centred at the System Reference Point. 2. The elevation shall be given with respect to the local WGS-84 tangential plane of the receiver dedicated by I015/400. 3. If populated, the range contour requires a start and stop point. The stop point is to be greater or equal than the start point.

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-631-001: Parse valid Contour (Azimuth, Elevation Angle, Range Extent)
- TC-15-631-002: Verify format compliance
- TC-15-631-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-SP: Parse Special Purpose Field (I15/SP)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
SP

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-SP-001: Parse valid Special Purpose Field
- TC-15-SP-002: Verify format compliance
- TC-15-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-15-RE: Parse Reserved Field (I15/RE)

**Parent:** REQ-HLR-CAT-15
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I15/RE (Reserved Field) as specified in the EUROCONTROL ASTERIX CAT15 v1.2 specification.

**Data Item Definition:**
RE

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-RE-001: Parse valid Reserved Field
- TC-15-RE-002: Verify format compliance
- TC-15-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat015_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-15-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT15 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-ERR-001: Truncated data items
- TC-15-ERR-002: Invalid repetition counts
- TC-15-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-15-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT15 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-15-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-15-000 | I15/000 | Message Type | Critical | High |
| REQ-LLR-15-010 | I15/010 | Data Source Identifier | Critical | High |
| REQ-LLR-15-015 | I15/015 | Service Identification | Medium | Low |
| REQ-LLR-15-020 | I15/020 | Target Report Descriptor | Critical | High |
| REQ-LLR-15-030 | I15/030 | Warning/Error Conditions | Medium | Low |
| REQ-LLR-15-050 | I15/050 | Update Period | Medium | Low |
| REQ-LLR-15-145 | I15/145 | Time of Applicability | Critical | High |
| REQ-LLR-15-161 | I15/161 | Track/Plot Number | Critical | High |
| REQ-LLR-15-170 | I15/170 | Track/Plot Status | Critical | High |
| REQ-LLR-15-270 | I15/270 | Target Size and Orientation | Medium | Low |
| REQ-LLR-15-300 | I15/300 | Object Classification | Medium | Low |
| REQ-LLR-15-400 | I15/400 | Measurement Identifier | Medium | Low |
| REQ-LLR-15-480 | I15/480 | Associations | Medium | Low |
| REQ-LLR-15-600 | I15/600 | Horizontal Position Information | Medium | Low |
| REQ-LLR-15-601 | I15/601 | Geometric Height Information | Medium | Low |
| REQ-LLR-15-602 | I15/602 | Horizontal Velocity Information | Medium | Low |
| REQ-LLR-15-603 | I15/603 | Horizontal Acceleration Information | Medium | Low |
| REQ-LLR-15-604 | I15/604 | Vertical Velocity Information | Medium | Low |
| REQ-LLR-15-605 | I15/605 | Vertical Acceleration Information | Medium | Low |
| REQ-LLR-15-625 | I15/625 | Range Information | Medium | Low |
| REQ-LLR-15-626 | I15/626 | Doppler Information | Medium | Low |
| REQ-LLR-15-627 | I15/627 | Azimuth Information | Medium | Low |
| REQ-LLR-15-628 | I15/628 | Elevation Information | Medium | Low |
| REQ-LLR-15-630 | I15/630 | Path Quality | Medium | Low |
| REQ-LLR-15-631 | I15/631 | Contour (Azimuth, Elevation Angle, Range | Medium | Low |
| REQ-LLR-15-SP | I15/SP | Special Purpose Field | Medium | Low |
| REQ-LLR-15-RE | I15/RE | Reserved Field | Medium | Low |
| REQ-LLR-15-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-15-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 29

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-15
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-15-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT15 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
