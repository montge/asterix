# Low-Level Requirements - ASTERIX Category 021
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT021-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-021
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 021 (Surveillance Data Exchange - Part 12 ADS-B Reports) v2.6.

**Parent Requirement:** REQ-HLR-CAT-021 - Parse ASTERIX Category 021

Each data item in CAT021 has corresponding LLRs defining parsing behavior.

---

## 2. CAT021 Data Item Requirements

### REQ-LLR-021-008: Parse Aircraft Operational Status (I021/008)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/008 (Aircraft Operational Status) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Identification of the operational services available in the aircraft while airborne.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-008-001: Parse valid Aircraft Operational Status
- TC-021-008-002: Verify format compliance
- TC-021-008-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-010: Parse Data Source Identification (I021/010)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I021/010 (Data Source Identification) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Identification of the ADS-B station providing information.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: mandatory
- Note: The up-to-date list of SACs is published on the EUROCONTROL ASTERIX Web Site (http://www.eurocontrol.int/services/system-area-code-list).

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-010-001: Parse valid Data Source Identification
- TC-021-010-002: Verify format compliance
- TC-021-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-021-015: Parse Service Identification (I021/015)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/015 (Service Identification) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Identification of the service provided to one or more users.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
            1- The service identification is allocated by the system.
            2- The service identification is also available in item I023/015 [Ref. 3].
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-015-001: Parse valid Service Identification
- TC-021-015-002: Verify format compliance
- TC-021-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-016: Parse Service Management (I021/016)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/016 (Service Management) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Identification of services offered by a ground station (identified by a SIC code).

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 1. This item contains the same information as item I023/101 in ASTERIX category 023 [Ref. 3]. Since not all service users receive category 023 data, this information has to be conveyed in category 021 as well. 2. If this item is due to be sent according to the encoding rule above, it shall be sent with the next target report

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-016-001: Parse valid Service Management
- TC-021-016-002: Verify format compliance
- TC-021-016-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-020: Parse Emitter Category (I021/020)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/020 (Emitter Category) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Characteristics of the originating ADS-B unit.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-020-001: Parse valid Emitter Category
- TC-021-020-002: Verify format compliance
- TC-021-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-040: Parse Target Report Descriptor (I021/040)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I021/040 (Target Report Descriptor) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Type and characteristics of the data as transmitted by a system.

**Implementation Notes:**
- Format: Variable
- Rule: mandatory
- Note: 
            1. Bit 3 indicates that the position reported by the target is within a credible range from the ground station. The range check is followed by the CPR validation to ensure that global and local position decoding both indicate valid position information. Bit 3=1 indicates that the range check was done, but the CPR validation is not yet completed. Once CPR validation is completed, Bit 3 will be reset to 0.
            2. The second extension signals the reasons for which the report has been indicated as suspect (indication Confidence Level (CL) in the first extension). Bit 2 indicates that the Range Check failed, i.e. the target is reported outside the credible range for the Ground Station. For operational users such a target will be suppressed. In services used for monitoring the Ground Station, the target will be transmitted with bit 2 indicating the fault condition.
            3. Bit 6, if set to 1, indicates that the position reported by the target was validated by an independent means and a discrepancy was ADS-B Target Reports EUROCONTROL-SPEC-0149-12 Edition : 2.4 Released Edition Page 17 detected. If no independent position check is implemented, the default value '0' is to be used.
            4. Bit 5 represents the setting of the GO/NOGO-bit as defined in item I023/100 of category 023 [Ref. 3].
            5. Bit 7, if set to 1, indicates that a lookup in a Black-list/White-list failed, indicating that the target may be suspect
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-040-001: Parse valid Target Report Descriptor
- TC-021-040-002: Verify format compliance
- TC-021-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-021-070: Parse Mode 3/A Code in Octal Representation (I021/070)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/070 (Mode 3/A Code in Octal Representation) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Mode-3/A code converted into octal representation.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-070-001: Parse valid Mode 3/A Code in Octal Representation
- TC-021-070-002: Verify format compliance
- TC-021-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-071: Parse Time of Applicability for Position (I021/071)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/071 (Time of Applicability for Position) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Time of applicability of the reported position, in the form of elapsed time since last midnight, expressed as UTC.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: 1. The time of applicability value is reset to zero at every midnight. 2. The time of applicability indicates the exact time at which the position transmitted in the target report is valid.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-071-001: Parse valid Time of Applicability for Position
- TC-021-071-002: Verify format compliance
- TC-021-071-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-072: Parse Time of Applicability for Velocity (I021/072)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/072 (Time of Applicability for Velocity) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Time of applicability (measurement) of the reported velocity, in the form of elapsed time since last midnight, expressed as UTC.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: 1. The time of applicability value is reset to zero at every midnight. 2. The time of applicability indicates the exact time at which the velocity information transmitted in the target report is valid. 3. This item will not be available in some ADS-B technologies.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-072-001: Parse valid Time of Applicability for Velocity
- TC-021-072-002: Verify format compliance
- TC-021-072-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-073: Parse Time of Message Reception for Position (I021/073)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/073 (Time of Message Reception for Position) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Time of reception of the latest position squitter in the Ground Station, in the form of elapsed time since last midnight, expressed as UTC.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: The time of message reception value is reset to zero at every midnight.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-073-001: Parse valid Time of Message Reception for Position
- TC-021-073-002: Verify format compliance
- TC-021-073-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-074: Parse Time of Message Reception of Position-High Precision (I021/074)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/074 (Time of Message Reception of Position-High Precision) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Time at which the latest ADS-B position information was received by the ground station, expressed as fraction of the second of the UTC Time.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-074-001: Parse valid Time of Message Reception of Position-High Precision
- TC-021-074-002: Verify format compliance
- TC-021-074-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-075: Parse Time of Message Reception for Velocity (I021/075)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/075 (Time of Message Reception for Velocity) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Time of reception of the latest velocity squitter in the Ground Station, in the form of elapsed time since last midnight, expressed as UTC.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: The time of message reception value is reset to zero at every midnight.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-075-001: Parse valid Time of Message Reception for Velocity
- TC-021-075-002: Verify format compliance
- TC-021-075-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-076: Parse Time of Message Reception of Velocity-High Precision (I021/076)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/076 (Time of Message Reception of Velocity-High Precision) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Time at which the latest ADS-B velocity information was received by the ground station, expressed as fraction of the second of the UTC Time.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-076-001: Parse valid Time of Message Reception of Velocity-High Precision
- TC-021-076-002: Verify format compliance
- TC-021-076-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-077: Parse Time of ASTERIX Report Transmission (I021/077)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/077 (Time of ASTERIX Report Transmission) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Time of the transmission of the ASTERIX category 021 report in the form of elapsed time since last midnight, expressed as UTC.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: The time of ASTERIX report transmission value is reset to zero at every midnight.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-077-001: Parse valid Time of ASTERIX Report Transmission
- TC-021-077-002: Verify format compliance
- TC-021-077-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-080: Parse Target Address (I021/080)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I021/080 (Target Address) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Target address (emitter identifier) assigned uniquely to each target.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: mandatory

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-080-001: Parse valid Target Address
- TC-021-080-002: Verify format compliance
- TC-021-080-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-021-090: Parse Quality Indicators (I021/090)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I021/090 (Quality Indicators) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
ADS-B quality indicators transmitted by a/c according to MOPS version.

**Implementation Notes:**
- Format: Variable
- Rule: mandatory
- Note: 
1. The primary subfield is kept for backwards compatibility reasons. Version 2 NIC-values shall be mapped accordingly. This is required to ensure that downstream systems, which are not capable of interpreting extensions 2 and 3 (because they use an ASTERIX edition earlier than 2.0) still get the required information
2. 'Version 1' or 'Version 2' refers to the MOPS version as defined in data item I021/210, bits 6/4
3. PIC=0 is defined for completeness only. In this case the third extension shall not be generated.
4. For ED102A/DO260B PIC values of 7 and 9, the NIC supplements for airborne messages (NIC supplements A/B) and surface messages (NIC supplements A/C) are listed. For ED102A/DO260B PIC=8, the NIC supplements A/B for airborne messages are listed. For DO260A PIC values of 7 and 8, the NIC supplement for airborne messages is shown in brackets. The aircraft air-ground status, and hence message type (airborne or surface), is derived from the GBS-bit in I021/040, 1st extension
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-090-001: Parse valid Quality Indicators
- TC-021-090-002: Verify format compliance
- TC-021-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-021-110: Parse Trajectory Intent (I021/110)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/110 (Trajectory Intent) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Reports indicating the 4D intended trajectory of the aircraft.

**Implementation Notes:**
- Format: Compound
- Rule: optional
- Note: 
1. NC is set to one when the aircraft will not fly the path described by the TCP data.
2. TCP numbers start from zero.
3. LT = Lateral Type
4. VT = Vertical Type
5. TOV gives the estimated time before reaching the point. It is defined as the absolute time from midnight.
6. TOV is meaningful only if TOA is set to 1.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-110-001: Parse valid Trajectory Intent
- TC-021-110-002: Verify format compliance
- TC-021-110-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-130: Parse Position in WGS-84 Co-ordinates (I021/130)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/130 (Position in WGS-84 Co-ordinates) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Position in WGS-84 Co-ordinates.

**Implementation Notes:**
- Format: Fixed (6 bytes)
- Rule: optional
- Note: Positive longitude indicates East. Positive latitude indicates North.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-130-001: Parse valid Position in WGS-84 Co-ordinates
- TC-021-130-002: Verify format compliance
- TC-021-130-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-131: Parse High-Resolution Position in WGS-84 Co-ordinates (I021/131)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/131 (High-Resolution Position in WGS-84 Co-ordinates) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Position in WGS-84 Co-ordinates in high resolution.

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: optional
- Note: Positive longitude indicates East. Positive latitude indicates North.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-131-001: Parse valid High-Resolution Position in WGS-84 Co-ordinates
- TC-021-131-002: Verify format compliance
- TC-021-131-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-132: Parse Message Amplitude (I021/132)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/132 (Message Amplitude) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Amplitude, in dBm, of ADS-B messages received by the ground station, coded in two's complement.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-132-001: Parse valid Message Amplitude
- TC-021-132-002: Verify format compliance
- TC-021-132-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-140: Parse Geometric Height (I021/140)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/140 (Geometric Height) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Minimum height from a plane tangent to the earth's ellipsoid, defined by WGS-84, in two's complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            1. LSB is required to be less than 10 ft by ICAO.
            2. A value of '0111111111111111' indicates that the aircraft transmits a 'greater than' indication.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-140-001: Parse valid Geometric Height
- TC-021-140-002: Verify format compliance
- TC-021-140-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-145: Parse Flight Level (I021/145)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/145 (Flight Level) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Flight Level from barometric measurements, not QNH corrected, in two's complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-145-001: Parse valid Flight Level
- TC-021-145-002: Verify format compliance
- TC-021-145-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-146: Parse Intermediate State Selected Altitude (I021/146)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/146 (Intermediate State Selected Altitude) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
The Selected Altitude as provided by the avionics and corresponding either to the MCP/FCU Selected Altitude (the ATC cleared altitude entered by the flight crew into the avionics) or to the FMS Selected Altitude.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            1. The Selected Altitude provided in this field is not necessarily the 'Target Altitude' as defined by ICAO.
            2. The value of 'Source' (bits 15/14) indicating 'unknown' or 'Aircraft Altitude' is kept for backward compatibility as these indications are not provided by 'version 2' systems as defined by data item I021/210, bits 6/4.
            3. Vertical mode indications supporting the determination of the nature of the Selected Altitude are provided in the Reserved Expansion Field in the subfield NAV.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-146-001: Parse valid Intermediate State Selected Altitude
- TC-021-146-002: Verify format compliance
- TC-021-146-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-148: Parse Final State Selected Altitude (I021/148)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/148 (Final State Selected Altitude) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
The vertical intent value that corresponds with the ATC cleared altitude, as derived from the Altitude Control Panel (MCP/FCU).

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: This item is kept for backward compatibility but shall not be used for 'version 2' ADS-B systems (as defined by data item I021/210, bits 6/4) for which item 146 will be used to forward the MCP/FCU or the FMS selected altitude information. For 'version 2' ADS-B systems, the vertical mode indications will be provided through the Reserved Expansion Field in the subfield NAV .

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-148-001: Parse valid Final State Selected Altitude
- TC-021-148-002: Verify format compliance
- TC-021-148-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-150: Parse Air Speed (I021/150)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/150 (Air Speed) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Calculated Air Speed (Element of Air Vector).

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-150-001: Parse valid Air Speed
- TC-021-150-002: Verify format compliance
- TC-021-150-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-151: Parse True Air Speed (I021/151)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/151 (True Air Speed) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
True Air Speed.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The RE-Bit, if set, indicates that the value to be transmitted is beyond the range defined for this specific data item and the applied technology. In this case the True Air Speed contains the maximum value that can be downloaded from the aircraft avionics and the RE- bit indicates that the actual value is greater than the value contained in the field.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-151-001: Parse valid True Air Speed
- TC-021-151-002: Verify format compliance
- TC-021-151-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-152: Parse Magnetic Heading (I021/152)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/152 (Magnetic Heading) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Magnetic Heading (Element of Air Vector).

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: True North Heading is defined in the Reserved Expansion Field in the subfield TNH.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-152-001: Parse valid Magnetic Heading
- TC-021-152-002: Verify format compliance
- TC-021-152-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-155: Parse Barometric Vertical Rate (I021/155)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/155 (Barometric Vertical Rate) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Barometric Vertical Rate, in two's complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The RE-Bit, if set, indicates that the value to be transmitted is beyond the range defined for this specific data item and the applied technology. In this case the Barometric Vertical Rate contains the maximum value that can be downloaded from the aircraft avionics and the RE-bit indicates that the actual value is greater than the value contained in the field.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-155-001: Parse valid Barometric Vertical Rate
- TC-021-155-002: Verify format compliance
- TC-021-155-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-157: Parse Geometric Vertical Rate (I021/157)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/157 (Geometric Vertical Rate) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Geometric Vertical Rate, in two's complement form, with reference to WGS-84.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: The RE-Bit, if set, indicates that the value to be transmitted is beyond the range defined for this specific data item and the applied technology. In this case the Geometric Vertical Rate contains the maximum value that can be downloaded from the aircraft avionics and the RE-bit indicates that the actual value is greater than the value contained in the field.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-157-001: Parse valid Geometric Vertical Rate
- TC-021-157-002: Verify format compliance
- TC-021-157-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-160: Parse Airborne Ground Vector (I021/160)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/160 (Airborne Ground Vector) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Ground Speed and Track Angle elements of Ground Vector.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: 
            1. The RE-Bit, if set, indicates that the value to be transmitted is beyond the range defined for this specific data item and the applied technology. In this case the Ground Speed contains the maximum value that can be downloaded from the aircraft avionics and the RE- bit indicates that the actual value is greater than the value contained in the field.
            2. The Surface Ground Vector format is defined in the Reserved Expansion Field in the subfield SGV.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-160-001: Parse valid Airborne Ground Vector
- TC-021-160-002: Verify format compliance
- TC-021-160-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-161: Parse Track Number (I021/161)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/161 (Track Number) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
An integer value representing a unique reference to a track record within a particular track file.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-161-001: Parse valid Track Number
- TC-021-161-002: Verify format compliance
- TC-021-161-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-165: Parse Track Angle Rate (I021/165)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/165 (Track Angle Rate) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Rate of Turn, in two's complement form.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            1. A positive value represents a right turn, whereas a negative value represents a left turn.
            2. 'Maximum value' means Maximum value or above.
            3. This item will not be transmitted for the technology '1090 MHz Extended Squitter'.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-165-001: Parse valid Track Angle Rate
- TC-021-165-002: Verify format compliance
- TC-021-165-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-170: Parse Target Identification (I021/170)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/170 (Target Identification) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Target (aircraft or vehicle) identification in 8 characters, as reported by the target.

**Implementation Notes:**
- Format: Fixed (6 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-170-001: Parse valid Target Identification
- TC-021-170-002: Verify format compliance
- TC-021-170-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-200: Parse Target Status (I021/200)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/200 (Target Status) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Status of the target

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: Bit-8 (ICF), when set to '1' indicates that new information is available in the Mode S GICB registers 40, 41 or 42.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-200-001: Parse valid Target Status
- TC-021-200-002: Verify format compliance
- TC-021-200-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-210: Parse MOPS Version (I021/210)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/210 (MOPS Version) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Identification of the MOPS version used by a/c to supply ADS-B information.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: Bit 7 (VNS) when set to 1 indicates that the aircraft transmits a MOPS Version indication that is not supported by the Ground Station. However, since MOPS versions are supposed to be backwards compatible, the GS has attempted to interpret the message and achieved a credible result. The fact that the MOPS version received is not supported by the GS is submitted as additional information to subsequent processing systems.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-210-001: Parse valid MOPS Version
- TC-021-210-002: Verify format compliance
- TC-021-210-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-220: Parse Met Information (I021/220)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/220 (Met Information) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Meteorological information.

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-220-001: Parse valid Met Information
- TC-021-220-002: Verify format compliance
- TC-021-220-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-230: Parse Roll Angle (I021/230)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/230 (Roll Angle) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
The roll angle, in two's complement form, of an aircraft executing a turn.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 1. Negative Value indicates 'Left Wing Down'. 2. Resolution provided by the technology '1090 MHz Extended Squitter' is 1 degree.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-230-001: Parse valid Roll Angle
- TC-021-230-002: Verify format compliance
- TC-021-230-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-250: Parse Mode S MB Data (I021/250)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I021/250 (Mode S MB Data) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Mode S Comm B data as extracted from the aircraft transponder.

**Implementation Notes:**
- Format: Repetitive
- Rule: mandatory
- Note: 
            1. For the transmission of BDS20, item 170 should be used.
            2. For the transmission of BDS30, item 260 is used.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-250-001: Parse valid Mode S MB Data
- TC-021-250-002: Verify format compliance
- TC-021-250-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-021-260: Parse ACAS Resolution Advisory Report (I021/260)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** Critical

**Description:**
The parser shall extract Data Item I021/260 (ACAS Resolution Advisory Report) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
RA

**Implementation Notes:**
- Format: Fixed (7 bytes)
- Rule: mandatory
- Note: 
1. Version denotes the MOPS version as defined in I021/210, bits 6/4
2. This data items copies the value of BDS register 6,1 for message type 28, subtype 2
3. The 'TYP' and 'STYP' items are implementation (i.e. link technology) dependent.
4. Refer to ICAO Annex 10 SARPs for detailed explanations [Ref. 10].
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-260-001: Parse valid ACAS Resolution Advisory Report
- TC-021-260-002: Verify format compliance
- TC-021-260-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** High

---

### REQ-LLR-021-271: Parse Surface Capabilities and Characteristics (I021/271)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/271 (Surface Capabilities and Characteristics) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Operational capabilities of the aircraft while on the ground.

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            1. Version 2 (as defined in I021/210, bits 6/4) data technology protocols encode 'No Data or Unknown' with value 0. In this case data item I021/271, first extension is not generated.
            2. As of edition 2.2 the structure of this data item has been changed. Edition 2.2 is not backwards compatible with previous editions.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-271-001: Parse valid Surface Capabilities and Characteristics
- TC-021-271-002: Verify format compliance
- TC-021-271-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-295: Parse Data Ages (I021/295)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/295 (Data Ages) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Ages of the data provided.

**Implementation Notes:**
- Format: Compound
- Rule: optional
- Note: 1. In all the subfields, the maximum value indicates 'maximum value or above'.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-295-001: Parse valid Data Ages
- TC-021-295-002: Verify format compliance
- TC-021-295-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-400: Parse Receiver ID (I021/400)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/400 (Receiver ID) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Designator of Ground Station in Distributed System.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-400-001: Parse valid Receiver ID
- TC-021-400-002: Verify format compliance
- TC-021-400-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-RE: Parse Reserved Expansion Field (I021/RE)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/RE (Reserved Expansion Field) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Reserved Expansion Field of ASTERIX Cat 021 (ADS-B Reports), Edition 1.5

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-RE-001: Parse valid Reserved Expansion Field
- TC-021-RE-002: Verify format compliance
- TC-021-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-021-SP: Parse Special Purpose (I021/SP)

**Parent:** REQ-HLR-CAT-021
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I021/SP (Special Purpose) as specified in the EUROCONTROL ASTERIX CAT021 v2.6 specification.

**Data Item Definition:**
Special information

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-SP-001: Parse valid Special Purpose
- TC-021-SP-002: Verify format compliance
- TC-021-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat021_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

## 3. Error Handling Requirements

### REQ-LLR-021-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT021 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-ERR-001: Truncated data items
- TC-021-ERR-002: Invalid repetition counts
- TC-021-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-021-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT021 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-021-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-021-008 | I021/008 | Aircraft Operational Status | High | Medium |
| REQ-LLR-021-010 | I021/010 | Data Source Identification | Critical | High |
| REQ-LLR-021-015 | I021/015 | Service Identification | High | Medium |
| REQ-LLR-021-016 | I021/016 | Service Management | High | Medium |
| REQ-LLR-021-020 | I021/020 | Emitter Category | High | Medium |
| REQ-LLR-021-040 | I021/040 | Target Report Descriptor | Critical | High |
| REQ-LLR-021-070 | I021/070 | Mode 3/A Code in Octal Representation | High | Medium |
| REQ-LLR-021-071 | I021/071 | Time of Applicability for Position | High | Medium |
| REQ-LLR-021-072 | I021/072 | Time of Applicability for Velocity | High | Medium |
| REQ-LLR-021-073 | I021/073 | Time of Message Reception for Position | High | Medium |
| REQ-LLR-021-074 | I021/074 | Time of Message Reception of Position-Hi | High | Medium |
| REQ-LLR-021-075 | I021/075 | Time of Message Reception for Velocity | High | Medium |
| REQ-LLR-021-076 | I021/076 | Time of Message Reception of Velocity-Hi | High | Medium |
| REQ-LLR-021-077 | I021/077 | Time of ASTERIX Report Transmission | High | Medium |
| REQ-LLR-021-080 | I021/080 | Target Address | Critical | High |
| REQ-LLR-021-090 | I021/090 | Quality Indicators | Critical | High |
| REQ-LLR-021-110 | I021/110 | Trajectory Intent | High | Medium |
| REQ-LLR-021-130 | I021/130 | Position in WGS-84 Co-ordinates | High | Medium |
| REQ-LLR-021-131 | I021/131 | High-Resolution Position in WGS-84 Co-or | High | Medium |
| REQ-LLR-021-132 | I021/132 | Message Amplitude | High | Medium |
| REQ-LLR-021-140 | I021/140 | Geometric Height | High | Medium |
| REQ-LLR-021-145 | I021/145 | Flight Level | High | Medium |
| REQ-LLR-021-146 | I021/146 | Intermediate State Selected Altitude | High | Medium |
| REQ-LLR-021-148 | I021/148 | Final State Selected Altitude | High | Medium |
| REQ-LLR-021-150 | I021/150 | Air Speed | High | Medium |
| REQ-LLR-021-151 | I021/151 | True Air Speed | High | Medium |
| REQ-LLR-021-152 | I021/152 | Magnetic Heading | High | Medium |
| REQ-LLR-021-155 | I021/155 | Barometric Vertical Rate | High | Medium |
| REQ-LLR-021-157 | I021/157 | Geometric Vertical Rate | High | Medium |
| REQ-LLR-021-160 | I021/160 | Airborne Ground Vector | High | Medium |
| REQ-LLR-021-161 | I021/161 | Track Number | High | Medium |
| REQ-LLR-021-165 | I021/165 | Track Angle Rate | High | Medium |
| REQ-LLR-021-170 | I021/170 | Target Identification | High | Medium |
| REQ-LLR-021-200 | I021/200 | Target Status | High | Medium |
| REQ-LLR-021-210 | I021/210 | MOPS Version | High | Medium |
| REQ-LLR-021-220 | I021/220 | Met Information | High | Medium |
| REQ-LLR-021-230 | I021/230 | Roll Angle | High | Medium |
| REQ-LLR-021-250 | I021/250 | Mode S MB Data | Critical | High |
| REQ-LLR-021-260 | I021/260 | ACAS Resolution Advisory Report | Critical | High |
| REQ-LLR-021-271 | I021/271 | Surface Capabilities and Characteristics | High | Medium |
| REQ-LLR-021-295 | I021/295 | Data Ages | High | Medium |
| REQ-LLR-021-400 | I021/400 | Receiver ID | High | Medium |
| REQ-LLR-021-RE | I021/RE | Reserved Expansion Field | High | Medium |
| REQ-LLR-021-SP | I021/SP | Special Purpose | High | Medium |
| REQ-LLR-021-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-021-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 46

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-021
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-021-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT021 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
