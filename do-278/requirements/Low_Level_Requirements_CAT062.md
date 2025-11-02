# Low-Level Requirements - ASTERIX Category 062
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT062-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-062
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 062 (SDPS Track Messages) v1.19.

**Parent Requirement:** REQ-HLR-CAT-062 - Parse ASTERIX Category 062

Each data item in CAT062 has corresponding LLRs defining parsing behavior.

---

## 2. CAT062 Data Item Requirements

### REQ-LLR-062-010: Parse Data Source Identifier (I062/010)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Identification of the system sending the data.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Note:
                - The up-to-date list of SACs is published on the
                  EUROCONTROL Web Site (http://www.eurocontrol.int/asterix).
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-010-001: Parse valid Data Source Identifier
- TC-062-010-002: Verify format compliance
- TC-062-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-015: Parse Service Identification (I062/015)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/015 (Service Identification) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Identification of the service provided to one or more users.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-015-001: Parse valid Service Identification
- TC-062-015-002: Verify format compliance
- TC-062-015-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-040: Parse Track Number (I062/040)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/040 (Track Number) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Identification of a track.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-040-001: Parse valid Track Number
- TC-062-040-002: Verify format compliance
- TC-062-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-060: Parse Track Mode 3/A Code (I062/060)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/060 (Track Mode 3/A Code) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Mode-3/A code converted into octal representation.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-060-001: Parse valid Track Mode 3/A Code
- TC-062-060-002: Verify format compliance
- TC-062-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-070: Parse Time Of Track Information (I062/070)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/070 (Time Of Track Information) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Absolute time stamping of the information provided
            in the track message, in the form of elapsed time since
            last mid night, expressed as UTC.
        

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: 
            Notes:

                1. This is the time of the track state vector.
                2. The time is reset to zero at every midnight.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-070-001: Parse valid Time Of Track Information
- TC-062-070-002: Verify format compliance
- TC-062-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-080: Parse Track Status (I062/080)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/080 (Track Status) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Status of a track.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            Notes:

                1. Track type and coasting can also be derived from I062/290 System Track Update Ages
                2. If the system supports the technology, default value (0) means that the technology was used to produce the report
                3. If the system does not support the technology, default value is meaningless.
                4. Bits (EMS): other than subfield #11 of data item I062/380, these
                   bits allow the SDPS to set the emergency indication as derived from
                   other sources than ADS-B (e.g. based on the Mode 3/A code).
                5. Bit 3 (PFT): with this flag an SDPS can indicate that internal processing
                   points to the track being potentially false. Details on the internal
                   processing are system dependent. In order to improve security on
                   targets provided by ADS-B numerous validation functions have been
                   developed in the ADS-B ground domain. If any of these validation
                   functions show a potentially spoofed target, the PFT bit will be used to
                   convey this information to the CWP. If and how this information is
                   processed and displayed on the CWP is a local matter and not subject
                   to the category 062 specification.
                6. Bit (FPLT): this bit - if set - indicates that the information contained
                   in the target report has been updated by flight plan related data
                   because no surveillance data was available for the target, or was
                   created based on flight plan related data in areas with no
                   surveillance.
                7. Bit (DUPT) is set to 1 if the correlation between the target report and a flight
                   plan is not possible because the Mode 3/A code stated in the flight plan exists
                   more than once in the surveillance data.
                8. Bit (DUPF) - if set to 1 - indicates that for a specific surveillance target more
                   than one flight plan exists which makes correlation impossible.
                9. Bit (DUPM) is set to 1 if a target was correlated manually but also a regular
                   flight plan exists.
                10. All tracks for which bits 8, 7 or 6 are set to 1 are marked on the CWP.
                11. Bit 5 (SFC) is set to 1 when the SDPS considers the target to be on the Surface
                    (the actual meaning is implementation dependent - please refer to chapter 4.8
                    above).
                12. Bit 4 (IDD) is set to 1 when the Flight ID is present more than once in the
                    surveillance area.
                13. Bit 3 (IEC) is set to 1 when the comparison between various sources has
                    revealed an inconsistency in the information contained about emergency codes.
                14. If I062/080 (MRH) indicates &quot;Barometric altitude (Mode C) more
                    reliable&quot;, and a calculated altitude is transmitted, it shall be transmitted
                    using data item I062/135 Calculated Track Barometric Altitude.
                15. If I062/080 (MRH) indicates &quot;Geometric altitude more reliable&quot;, and a
                    calculated altitude is transmitted, it shall be transmitted using data item
                    I062/130 Calculated Track Geometric Altitude. In this case the source
                    for I062/130 is indicated by I062/080 (SRC).
                16. Data Items I062/130, I062/135, and I062/136 may be transmitted in
                    parallel whenever the respective information is available. This is
                    independent from the value transmitted on I062/080 (MRH).
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-080-001: Parse valid Track Status
- TC-062-080-002: Verify format compliance
- TC-062-080-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-100: Parse Calculated Track Position (Cartesian) (I062/100)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/100 (Calculated Track Position (Cartesian)) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Calculated position in Cartesian co-ordinates with a resolution of
            0.5m, in two's complement form.
        

**Implementation Notes:**
- Format: Fixed (6 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-100-001: Parse valid Calculated Track Position (Cartesian)
- TC-062-100-002: Verify format compliance
- TC-062-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-105: Parse Calculated Position In WGS-84 Co-ordinates (I062/105)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/105 (Calculated Position In WGS-84 Co-ordinates) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Calculated Position in WGS-84 Co-ordinates with a resolution of
            :math:`180/2^{25}` degrees.
        

**Implementation Notes:**
- Format: Fixed (8 bytes)
- Rule: optional
- Note: 
            Notes:

                - The LSB provides a resolution at least better than 0.6m.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-105-001: Parse valid Calculated Position In WGS-84 Co-ordinates
- TC-062-105-002: Verify format compliance
- TC-062-105-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-110: Parse Mode 5 Data Reports and Extended Mode 1 Code (I062/110)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/110 (Mode 5 Data Reports and Extended Mode 1 Code) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Mode 5 Data reports and Extended Mode 1 Code.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional
- Note: 
            Notes:

                1. The flags M2, M3, MC refer to the contents of data subitems I062/120,
                   I062/060 and I062/135 respectively. The flag M1 refers to the contents
                   of the Subfield #5 (Extended Mode 1 Code in Octal Representation).
                2. If an authenticated Mode 5 reply is received with the Emergency
                   bit set, then the Military Emergency bit (ME) in Data Item I062/080,
                   Track Status, shall be set.
                3. If an authenticated Mode 5 reply is received with the Identification
                   of Position bit set, then the Special Position Identification
                   bit (SPI) in Data Item I062/080, Track Status, shall be set.
                4. The resolution implied by the LSB is better than the resolution
                   with which Mode 5 position reports are transmitted from aircraft
                   transponders using currently defined formats.
                5. GA is coded as a 14-bit two's complement binary number with
                   an LSB of 25 ft. irrespective of the setting of RES.
                6. The minimum value of GA that can be reported is -1000 ft.
                7. If Subfield #1 is present, the M1 bit in Subfield #1 indicates
                   whether the Extended Mode 1 Code is from a Mode 5 reply or
                   a Mode 1 reply. If Subfield #1 is not present, the Extended
                   Mode 1 Code is from a Mode 1 reply.
                8. TOS shall be assumed to be zero if Subfield #6 is not present.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-110-001: Parse valid Mode 5 Data Reports and Extended Mode 1 Code
- TC-062-110-002: Verify format compliance
- TC-062-110-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-120: Parse Track Mode 2 Code (I062/120)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/120 (Track Mode 2 Code) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Mode 2 code associated to the track
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-120-001: Parse valid Track Mode 2 Code
- TC-062-120-002: Verify format compliance
- TC-062-120-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-130: Parse Calculated Track Geometric Altitude (I062/130)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/130 (Calculated Track Geometric Altitude) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Vertical distance between the target and the projection of its position
            on the earth's ellipsoid, as defined by WGS84, in two's complement form.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Notes:

                1. LSB is required to be less than 10 ft by ICAO
                2. The source of altitude is identified in bits (SRC) of item
                   I062/080 Track Status.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-130-001: Parse valid Calculated Track Geometric Altitude
- TC-062-130-002: Verify format compliance
- TC-062-130-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-135: Parse Calculated Track Barometric Altitude (I062/135)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/135 (Calculated Track Barometric Altitude) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Calculated barometric altitude of the track, in two's complement form.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Notes:

                1) ICAO specifies a range between -10 FL and 1267 FL for Mode C
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-135-001: Parse valid Calculated Track Barometric Altitude
- TC-062-135-002: Verify format compliance
- TC-062-135-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-136: Parse Measured Flight Level (I062/136)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/136 (Measured Flight Level) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Last valid and credible flight level used to update the track, in two's
            complement form.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Notes:

                1. The criteria to determine the credibility of the flight level
                   are Tracker dependent.
                2. Credible means: within reasonable range of change with respect
                   to the previous detection.
                3. ICAO specifies a range between -10 FL and 1267 FL for Mode C.
                4. This item includes the barometric altitude received from ADS-B.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-136-001: Parse valid Measured Flight Level
- TC-062-136-002: Verify format compliance
- TC-062-136-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-185: Parse Calculated Track Velocity (Cartesian) (I062/185)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/185 (Calculated Track Velocity (Cartesian)) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Calculated track velocity expressed in Cartesian co-ordinates,in
            two's complement form.
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: 
            Notes:

                - The y-axis points to the Geographical North at the location of
                  the target.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-185-001: Parse valid Calculated Track Velocity (Cartesian)
- TC-062-185-002: Verify format compliance
- TC-062-185-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-200: Parse Mode of Movement (I062/200)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/200 (Mode of Movement) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Calculated Mode of Movement of a target.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
            Notes:

                - The ADF, if set, indicates that a difference has been detected
                  in the altitude information derived from radar as compared to
                  other technologies (such as ADS-B).
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-200-001: Parse valid Mode of Movement
- TC-062-200-002: Verify format compliance
- TC-062-200-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-210: Parse Calculated Acceleration (Cartesian) (I062/210)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/210 (Calculated Acceleration (Cartesian)) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Calculated Acceleration of the target expressed in Cartesian co-ordinates,
            in two's complement form.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Notes:

                1. The y-axis points to the Geographical North at the location of the target.
                2. Maximum value means maximum value or above.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-210-001: Parse valid Calculated Acceleration (Cartesian)
- TC-062-210-002: Verify format compliance
- TC-062-210-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-220: Parse Calculated Rate of Climb/Descent (I062/220)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/220 (Calculated Rate of Climb/Descent) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Calculated rate of climb/descent of an aircraft in two's complement form.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Notes:

                1. A positive value indicates a climb, whereas a negative value
                   indicates a descent.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-220-001: Parse valid Calculated Rate of Climb/Descent
- TC-062-220-002: Verify format compliance
- TC-062-220-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-245: Parse Target Identification (I062/245)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/245 (Target Identification) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Target (aircraft or vehicle) identification in 8 characters.
        

**Implementation Notes:**
- Format: Fixed (7 bytes)
- Rule: optional
- Note: 
            Notes:

                1. For coding, see section 3.1.2.9 of [Ref.3]
                2. As the Callsign of the target can already be transmitted
                   (thanks to I062/380 Subfield #2 if downlinked from the
                   aircraft or thanks to I062/390 Subfield #2 if the target
                   is correlated to a flight plan), and in order to avoid
                   confusion at end user's side, this item SHALL not be used.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-245-001: Parse valid Target Identification
- TC-062-245-002: Verify format compliance
- TC-062-245-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-270: Parse Target Size and Orientation (I062/270)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/270 (Target Size and Orientation) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Target size defined as length and width of the detected target, and orientation.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            Notes:

                1. The orientation gives the direction which the target nose is
                   pointing to,relative to the Geographical North.
                2. When the length only is sent, the largest dimension is provided.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-270-001: Parse valid Target Size and Orientation
- TC-062-270-002: Verify format compliance
- TC-062-270-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-290: Parse System Track Update Ages (I062/290)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/290 (System Track Update Ages) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Ages of the last plot/local track/target report update for each sensor type.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional
- Note: 
            Notes:

                1. Except for Track Age, the ages are counted from Data Item I062/070,
                   Time Of Track Information, using the following formula:
                   Age = Time of track information - Time of last detection used
                   to update the track
                2. The time of last detection is derived from monosensor category time of day
                3. If the data has never been received, then the corresponding
                   subfield is not sent.
                4. Maximum value means maximum value or above.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-290-001: Parse valid System Track Update Ages
- TC-062-290-002: Verify format compliance
- TC-062-290-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-295: Parse Track Data Ages (I062/295)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/295 (Track Data Ages) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Ages of the data provided.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional
- Note: 
            Notes:

                1. Despite there are now two subfields (#29 and #30) reporting the ages
                   of, respectively, the Indicated Airspeed track data and the Mach
                   Number track data, the subfield #8 (and so its presence bit , bit-32) is
                   kept free in order to prevent a full incompatibility with previous
                   releases of ASTERIX Cat. 062 already implemented.
                2. In all the subfields, the age is the time delay since the value was
                   measured
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-295-001: Parse valid Track Data Ages
- TC-062-295-002: Verify format compliance
- TC-062-295-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-300: Parse Vehicle Fleet Identification (I062/300)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/300 (Vehicle Fleet Identification) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Vehicle fleet identification number.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-300-001: Parse valid Vehicle Fleet Identification
- TC-062-300-002: Verify format compliance
- TC-062-300-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-340: Parse Measured Information (I062/340)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/340 (Measured Information) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            All measured data related to the last report used to update the track.
            These data are not used for ADS-B.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional
- Note: 
            Notes:

                1. In case of a plot, the measured bias-corrected polar co-ordinates;
                2. In case of a sensor local track, the measured bias-corrected
                   polar co-ordinates of the plot associated to the track;
                3. In case of a local track without detection, the extrapolated
                   bias-corrected polar co-ordinates.
                4. Smoothed MODE 3/A data (L = 1) will be used in case of absence of
                   MODE 3/A code information in the plot or in case of difference
                   between plot and sensor local track MODE 3/A code information.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-340-001: Parse valid Measured Information
- TC-062-340-002: Verify format compliance
- TC-062-340-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-380: Parse Aircraft Derived Data (I062/380)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/380 (Aircraft Derived Data) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Data derived directly by the aircraft.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional
- Note: 
            Notes:

                1. NC is set to one when the aircraft will not fly the path described
                   by the TCP data.
                2. TCP numbers start from zero.
                3. LT = Lateral Type
                4. VT = Vertical Type
                5. TOV gives the estimated time before reaching the point. It is
                   defined as the absolute time from midnight.
                6. TOV is meaningful only if TOA is set to 0
                7. Refer to ICAO Draft SARPs for ACAS for detailed explanations.
                8. A positive value represents a right turn, whereas a negative value
                   represents a left turn.
                9. Value 15 means 15 degrees/s or above.
                10. Velocity uncertainty category of the least accurate velocity component
                11. Positive longitude indicates East. Positive latitude indicates North.
                12. LSB is required to be thinner than 10 ft by ICAO
                13. Only DAPs that can not be encoded into other subfields of this item
                    should be sent using subfield #25
                14. BPS is the barometric pressure setting of the aircraft minus 800 mb.
                15. As of Edition 1.19 the note &quot;(derived from Mode S BDS 4,0)&quot; has been
                    removed to allow transmission of BPS received via ADS-B.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-380-001: Parse valid Aircraft Derived Data
- TC-062-380-002: Verify format compliance
- TC-062-380-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-390: Parse Flight Plan Related Data (I062/390)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/390 (Flight Plan Related Data) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            All flight plan related information, provided by ground-based systems.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional
- Note: 
            Notes:

                1. The up-to-date list of SACs is published on the Eurocontrol Web Site
                   (http://www.eurocontrol.int).
                2. Each one of the seven Octets contains an ASCII Character.
                   TheCallsign is always left adjusted. It contains up to seven
                   upper-case alphanumeric characters, the remaining character
                   positions (if any)are padded with space characters.
                3. Each one of the four Octets composing the type of an aircraft
                   contains an ASCII Character (upper-case alphanumeric characters
                   with trailing spaces).
                4. The types of aircraft are defined in [Ref.4]
                5. Each one of the four Octets composing the name of an airport
                   contains an ASCII Character (upper case alphabetic).
                6. The Airport Names are indicated in the ICAO Location Indicators book.
                7. Each one of the four Octets composing the name of an airport
                   contains an ASCII Character (upper case alphabetic).
                8. The Airport Names are indicated in the ICAO Location Indicators book.
                9. NU1, NU2 and LTR each contain an ASCII character
                10. For details refer to.[5] Section 5
                11. The centre and the control position identification codes have to be
                    defined between communication partners.
                12. Estimated times are derived from flight plan systems. Predicted
                    times are derived by the fusion system, based on surveillance
                    data. For definitions, see [Ref.4]
                13. Each one of the six Octets contains an ASCII Character. The Aircraft
                    Stand identification is always left adjusted. It contains up
                    to six upper-case alphanumeric characters, the remaining character
                    positions (if any) are padded with space characters.
                14. Each one of the seven Octets contains an ASCII Character.
                    The SID is always left adjusted. It contains up to seven
                    alphanumeric characters, the remaining character positions
                    (if any) are padded with space characters.
                15. Each one of the seven Octets contains an ASCII Character.
                    The STAR is always left adjusted. It contains up to seven
                    alphanumeric characters, the remaining character positions
                    (if any) are padded with space characters.
                16. This subfield is used only when the aircraft is transmitting
                    an emergency Mode 3/A code
                17. If VA = 0, the content of bits 12/1 is meaningless
                18. Each one of the seven Octets contains an ASCII Character.
                    The Callsign is always left adjusted. It contains up to seven
                    upper-case alphanumeric characters, the remaining character
                    positions (if any) are padded with space characters
                19. This subfield is used only when an emergency Mode 3/A is associated
                    with the track (I062/390 Subfield #17)
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-390-001: Parse valid Flight Plan Related Data
- TC-062-390-002: Verify format compliance
- TC-062-390-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-500: Parse Estimated Accuracies (I062/500)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/500 (Estimated Accuracies) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Overview of all important accuracies.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional
- Note: 
            Notes:

                1. Maximum value means maximum value or above.
                2. XY covariance component = sign {Cov(X,Y)} * sqrt {abs [Cov (X,Y)]}
                3. The maximum value for the (unsigned) XY covariance component is 16.383 km
                4. Maximum value means maximum value or above.
                5. Maximum value means maximum value or above.
                6. Maximum value means maximum value or above.
                7. Maximum value means maximum value or above.
                8. Maximum value means maximum value or above.
                9. Maximum value means maximum value or above.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-500-001: Parse valid Estimated Accuracies
- TC-062-500-002: Verify format compliance
- TC-062-500-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-510: Parse Composed Track Number (I062/510)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/510 (Composed Track Number) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Identification of a system track.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            Notes:

                - The composed track number is used by co-operating units to uniquely
                  identify a track. It consists of the unit identifier and system
                  track number for each unit involved in the co-operation. The first
                  unit identification identifies the unit that is responsible for the
                  track amalgamation.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-510-001: Parse valid Composed Track Number
- TC-062-510-002: Verify format compliance
- TC-062-510-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-RE: Parse Reserved Expansion Field (I062/RE)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/RE (Reserved Expansion Field) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Expansion
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-RE-001: Parse valid Reserved Expansion Field
- TC-062-RE-002: Verify format compliance
- TC-062-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-062-SP: Parse Special Purpose Field (I062/SP)

**Parent:** REQ-HLR-CAT-062
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I062/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT062 v1.19 specification.

**Data Item Definition:**

            Special Purpose Field
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-SP-001: Parse valid Special Purpose Field
- TC-062-SP-002: Verify format compliance
- TC-062-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat062_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

## 3. Error Handling Requirements

### REQ-LLR-062-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT062 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-ERR-001: Truncated data items
- TC-062-ERR-002: Invalid repetition counts
- TC-062-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-062-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT062 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-062-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-062-010 | I062/010 | Data Source Identifier | High | Medium |
| REQ-LLR-062-015 | I062/015 | Service Identification | High | Medium |
| REQ-LLR-062-040 | I062/040 | Track Number | High | Medium |
| REQ-LLR-062-060 | I062/060 | Track Mode 3/A Code | High | Medium |
| REQ-LLR-062-070 | I062/070 | Time Of Track Information | High | Medium |
| REQ-LLR-062-080 | I062/080 | Track Status | High | Medium |
| REQ-LLR-062-100 | I062/100 | Calculated Track Position (Cartesian) | High | Medium |
| REQ-LLR-062-105 | I062/105 | Calculated Position In WGS-84 Co-ordinat | High | Medium |
| REQ-LLR-062-110 | I062/110 | Mode 5 Data Reports and Extended Mode 1  | High | Medium |
| REQ-LLR-062-120 | I062/120 | Track Mode 2 Code | High | Medium |
| REQ-LLR-062-130 | I062/130 | Calculated Track Geometric Altitude | High | Medium |
| REQ-LLR-062-135 | I062/135 | Calculated Track Barometric Altitude | High | Medium |
| REQ-LLR-062-136 | I062/136 | Measured Flight Level | High | Medium |
| REQ-LLR-062-185 | I062/185 | Calculated Track Velocity (Cartesian) | High | Medium |
| REQ-LLR-062-200 | I062/200 | Mode of Movement | High | Medium |
| REQ-LLR-062-210 | I062/210 | Calculated Acceleration (Cartesian) | High | Medium |
| REQ-LLR-062-220 | I062/220 | Calculated Rate of Climb/Descent | High | Medium |
| REQ-LLR-062-245 | I062/245 | Target Identification | High | Medium |
| REQ-LLR-062-270 | I062/270 | Target Size and Orientation | High | Medium |
| REQ-LLR-062-290 | I062/290 | System Track Update Ages | High | Medium |
| REQ-LLR-062-295 | I062/295 | Track Data Ages | High | Medium |
| REQ-LLR-062-300 | I062/300 | Vehicle Fleet Identification | High | Medium |
| REQ-LLR-062-340 | I062/340 | Measured Information | High | Medium |
| REQ-LLR-062-380 | I062/380 | Aircraft Derived Data | High | Medium |
| REQ-LLR-062-390 | I062/390 | Flight Plan Related Data | High | Medium |
| REQ-LLR-062-500 | I062/500 | Estimated Accuracies | High | Medium |
| REQ-LLR-062-510 | I062/510 | Composed Track Number | High | Medium |
| REQ-LLR-062-RE | I062/RE | Reserved Expansion Field | High | Medium |
| REQ-LLR-062-SP | I062/SP | Special Purpose Field | High | Medium |
| REQ-LLR-062-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-062-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 31

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-062
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-062-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT062 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
