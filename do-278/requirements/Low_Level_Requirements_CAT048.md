# Low-Level Requirements - ASTERIX Category 048
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT048-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-048
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 048 (Monoradar Target Reports) v1.30.

**Parent Requirement:** REQ-HLR-CAT-048 - Parse ASTERIX Category 048

Each data item in CAT048 has corresponding LLRs defining parsing behavior.

---

## 2. CAT048 Data Item Requirements

### REQ-LLR-048-010: Parse Data Source Identifier (I048/010)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Identification of the radar station from which the data is received.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Note:
                - The up-to-date list of SACs is published on the
                  EUROCONTROL Web Site (http://www.eurocontrol.int/asterix).
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-010-001: Parse valid Data Source Identifier
- TC-048-010-002: Verify format compliance
- TC-048-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-020: Parse Target Report Descriptor (I048/020)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/020 (Target Report Descriptor) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Type and properties of the target report.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            Notes:

                1. For Mode S aircraft, the SPI information is also contained in I048/230.
                2. To bits 3/2 (FOE/FRI): IFF interrogators supporting a three level
                   classification of the processing of the Mode 4 interrogation result
                   shall encode the detailed response information in data item M4E of
                   the Reserved Expansion Field of category 048. In this case the value
                   for FOE/FRI in I048/020 shall be set to 00.
                   However, even those interrogators shall use I048/020 to encode the information No reply.
                3. To bit 6 (XPP): This bit shall always be set when the X-pulse has
                   been extracted, independent from the Mode it was extracted with.
                4. To bit 7 (ERR): This bit set to 1 indicates that the range of the
                   target is beyond the maximum range in data item I048/040.In this
                   case - and this case only - the ERR Data Item in the Reserved
                   Expansion Field shall provide the range value of the Measured
                   Position in Polar Coordinates.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-020-001: Parse valid Target Report Descriptor
- TC-048-020-002: Verify format compliance
- TC-048-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-030: Parse Warning/Error Conditions and Target Classification (I048/030)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/030 (Warning/Error Conditions and Target Classification) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Warning/error conditions detected by a radar station for the target report
            involved. Target Classification information for the target involved.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            Notes:

                1. It has to be stressed that a series of one or more codes can
                   be reported per target report.
                2. Data conveyed in this item are of secondary importance, and
                   can generally also be derived from the processing of mandatory items.
                3. Definitions can be found in SUR.ET1.ST03.1000-STD-01-01 Radar
                   Sensor Performance Analysis.
                4. Values 25 to 30 have been defined to comply with the updated
                   European Mode S Specification (EMS) and to provide the possibility
                   to report the following information:

                       - Code 25: the maximum number of permitted re-interrogations to
                         acquire the surveillance information has been reached;
                       - Code 26: the maximum number of permitted re-interrogations to
                         extract BDS Registers has been reached;
                       - Code 27: inconsistency detected between the contents of the
                         message and the BDS register overlayed;
                       - Code 28: a BDS swap has been detected and the respective information
                         has been discarded;
                       - Code 29: the track has been updated while being in the zenithal
                         gap (also referred to as Cone of Silence);
                       - Code 30: the radar had lost track of an aircraft and subsequently
                         re-acquired it.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-030-001: Parse valid Warning/Error Conditions and Target Classification
- TC-048-030-002: Verify format compliance
- TC-048-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-040: Parse Measured Position in Polar Co-ordinates (I048/040)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/040 (Measured Position in Polar Co-ordinates) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Measured position of an aircraft in local polar co-ordinates.
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: 
            Notes:

                1. In case of no detection, the extrapolated position expressed in slant
                   polar co-ordinates may be sent, except for a track cancellation message.
                   No detection is signalled by the TYP field set to zero in I048/020
                   Target Report Descriptor.
                2. This item represents the measured target position of the plot, even
                   if associated with a track, for the present antenna scan. It is
                   expressed in polar co-ordinates in the local reference system,
                   centred on the radar station.
                3. In case of combined detection by a PSR and an SSR, then the SSR
                   position is sent.
                4. For targets having a range beyond the maximum range the data item
                   Extended Range Report has been added to the Reserved Expansion
                   Field of category 048. The presence of this data item is indicated
                   by the ERR bit set to one in data item I048/020, first extension.
                   The ERR data item shall only be sent if the value of RHO is equal
                   to or greater than 256NM.
                   Please note that if this data item is used, the Encoding Rule to
                   data item I048/040 still applies, meaning that the extra item in
                   the Reserved Expansion Field shall be transmitted in addition to
                   data item I048/040.
                   If the Extended Range Report item in the Reserved Expansion Field
                   is used, it is recommended to set the value of RHO in data item
                   I048/040 to its maximum, meaning bits 32/17 all set to 1.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-040-001: Parse valid Measured Position in Polar Co-ordinates
- TC-048-040-002: Verify format compliance
- TC-048-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-042: Parse Calculated Position in Cartesian Co-ordinates (I048/042)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/042 (Calculated Position in Cartesian Co-ordinates) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Calculated position of an aircraft in Cartesian co-ordinates.
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-042-001: Parse valid Calculated Position in Cartesian Co-ordinates
- TC-048-042-002: Verify format compliance
- TC-048-042-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-050: Parse Mode-2 Code in Octal Representation (I048/050)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/050 (Mode-2 Code in Octal Representation) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Reply to Mode-2 interrogation.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Note:
                - Bit 15 has no meaning in the case of a smoothed Mode-2 and is set
                  to 0 for a calculated track.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-050-001: Parse valid Mode-2 Code in Octal Representation
- TC-048-050-002: Verify format compliance
- TC-048-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-055: Parse Mode-1 Code in Octal Representation (I048/055)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/055 (Mode-1 Code in Octal Representation) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Reply to Mode-1 interrogation.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
            Notes:

                1. Bit 7 has no meaning in the case of a smoothed Mode-1 and is set
                   to 0 for a calculated track.
                2. The values of the bits for V, G, L, A4, A2, A1, B2 and B1 shall be
                   identical to the values of the corresponding bits in subfield #5
                   of data item MD5 - Mode 5 Reports and in subfield #5 of data
                   item MD5 - Mode 5 Reports, New Format in the Reserved Expansion Field.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-055-001: Parse valid Mode-1 Code in Octal Representation
- TC-048-055-002: Verify format compliance
- TC-048-055-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-060: Parse Mode-2 Code Confidence Indicator (I048/060)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/060 (Mode-2 Code Confidence Indicator) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Confidence level for each bit of a Mode-2 reply as provided by a monopulse SSR station.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-060-001: Parse valid Mode-2 Code Confidence Indicator
- TC-048-060-002: Verify format compliance
- TC-048-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-065: Parse Mode-1 Code Confidence Indicator (I048/065)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/065 (Mode-1 Code Confidence Indicator) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Confidence level for each bit of a Mode-1 reply as provided by a monopulse SSR station.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-065-001: Parse valid Mode-1 Code Confidence Indicator
- TC-048-065-002: Verify format compliance
- TC-048-065-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-070: Parse Mode-3/A Code in Octal Representation (I048/070)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/070 (Mode-3/A Code in Octal Representation) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Mode-3/A code converted into octal representation.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Notes:

                1. Bit 15 has no meaning in the case of a smoothed Mode-3/A code and
                   is set to 0 for a calculated track. For Mode S, it is set to one
                   when an error correction has been attempted.
                2. For Mode S, bit 16 is normally set to zero, but can exceptionally
                   be set to one to indicate a non-validated Mode-3/A code (e.g. alert
                   condition detected, but new Mode-3/A code not successfully extracted).
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-070-001: Parse valid Mode-3/A Code in Octal Representation
- TC-048-070-002: Verify format compliance
- TC-048-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-080: Parse Mode-3/A Code Confidence Indicator (I048/080)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/080 (Mode-3/A Code Confidence Indicator) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Confidence level for each bit of a Mode-3/A reply as provided by a monopulse SSR station.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-080-001: Parse valid Mode-3/A Code Confidence Indicator
- TC-048-080-002: Verify format compliance
- TC-048-080-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-090: Parse Flight Level in Binary Representation (I048/090)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/090 (Flight Level in Binary Representation) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Flight Level converted into binary representation.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Notes:

                1. When Mode C code / Mode S altitude code is present but not decodable,
                   the Undecodable Mode C code / Mode S altitude code Warning/Error
                   should be sent in I048/030.
                2. When local tracking is applied and the received Mode C code / Mode S
                   altitude code corresponds to an abnormal value (the variation with
                   the previous plot is estimated too important by the tracker),
                   the Mode C code / Mode S altitude code abnormal value compared
                   to the track Warning/Error should be sent in I048/030.
                3. The value shall be within the range described by ICAO Annex 10
                4. For Mode S, bit 15 (G) is set to one when an error correction has
                   been attempted.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-090-001: Parse valid Flight Level in Binary Representation
- TC-048-090-002: Verify format compliance
- TC-048-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-100: Parse Mode-C Code and Code Confidence Indicator (I048/100)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/100 (Mode-C Code and Code Confidence Indicator) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Mode-C height in Gray notation as received from the transponder together
            with the confidence level for each reply bit as provided by a MSSR/Mode S station.
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: 
            Notes:

                1. For Mode S, D1 is also designated as Q, and is used to denote either
                   25ft or 100ft reporting.
                2. For Mode S, bit-31 (G) is set when an error correction has been attempted.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-100-001: Parse valid Mode-C Code and Code Confidence Indicator
- TC-048-100-002: Verify format compliance
- TC-048-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-110: Parse Height Measured by a 3D Radar (I048/110)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/110 (Height Measured by a 3D Radar) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Height of a target as measured by a 3D radar. The height shall use mean
            sea level as the zero reference level.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-110-001: Parse valid Height Measured by a 3D Radar
- TC-048-110-002: Verify format compliance
- TC-048-110-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-120: Parse Radial Doppler Speed (I048/120)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/120 (Radial Doppler Speed) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Information on the Doppler Speed of the target report.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-120-001: Parse valid Radial Doppler Speed
- TC-048-120-002: Verify format compliance
- TC-048-120-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-130: Parse Radar Plot Characteristics (I048/130)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/130 (Radar Plot Characteristics) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Additional information on the quality of the target report.
        

**Implementation Notes:**
- Format: Compound
- Rule: optional
- Note: 
            Notes:

                1. The total range covered is therefore from 0 to 11.21 deg.
                2. Negative values are coded in two's complement form.
                3. The total range covered is therefore from 0 to 11.21 deg.
                4. Negative values are coded in two's complement form.
                5. Negative values are coded in two's complement form.
                6. The covered range difference is +/- 0.5 NM.
                7. Sending the maximum value means that the difference in range
                   is equal or greater than the maximum value.
                8. Negative values are coded in two's complement form.
                9. The covered azimuth difference is +/-360/2 7 = +/- 2.8125 deg.
                10. Sending the maximum value means that the difference in range
                    is equal or greater than the maximum value.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-130-001: Parse valid Radar Plot Characteristics
- TC-048-130-002: Verify format compliance
- TC-048-130-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-140: Parse Time of Day (I048/140)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/140 (Time of Day) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Absolute time stamping expressed as Co-ordinated Universal Time (UTC).
        

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: 
            Notes:

                1. The time of day value is reset to 0 each day at midnight.
                2. Every radar station using ASTERIX should be equipped with at least
                   one synchronised time source
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-140-001: Parse valid Time of Day
- TC-048-140-002: Verify format compliance
- TC-048-140-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-161: Parse Track Number (I048/161)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/161 (Track Number) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            An integer value representing a unique reference to a track record within
            a particular track file.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-161-001: Parse valid Track Number
- TC-048-161-002: Verify format compliance
- TC-048-161-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-170: Parse Track Status (I048/170)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/170 (Track Status) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Status of monoradar track (PSR and/or SSR updated).
        

**Implementation Notes:**
- Format: Variable
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-170-001: Parse valid Track Status
- TC-048-170-002: Verify format compliance
- TC-048-170-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-200: Parse Calculated Track Velocity in Polar Co-ordinates (I048/200)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/200 (Calculated Track Velocity in Polar Co-ordinates) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Calculated track velocity expressed in polar co-ordinates.
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: 
            Notes:

                - The calculated heading is related to the geographical North at the
                  aircraft position.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-200-001: Parse valid Calculated Track Velocity in Polar Co-ordinates
- TC-048-200-002: Verify format compliance
- TC-048-200-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-210: Parse Track Quality (I048/210)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/210 (Track Quality) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Track quality in the form of a vector of standard deviations.
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: 
            Notes:

                1. The standard deviation is per definition a positive value, hence
                   the range covered is : 0<= Sigma(X)<2 NM
                2. The standard deviation is per definition a positive value, hence
                   the range covered is : 0<= Sigma(Y)<2 NM
                3. The standard deviation is per definition a positive value, hence
                   the range covered is: 0<=Sigma (V)<56.25 Kt
                4. The standard deviation is per definition a positive value; hence
                   the range covered is: 0 <= sigma (H) < 22.5 degrees.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-210-001: Parse valid Track Quality
- TC-048-210-002: Verify format compliance
- TC-048-210-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-220: Parse Aircraft Address (I048/220)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/220 (Aircraft Address) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Aircraft address (24-bits Mode S address) assigned uniquely to each aircraft.
        

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional
- Note: 
            Note:
                - The Encoding Rule for Data Item I048/220 has been relaxed in Edition
                  1.30 for the End of Track Message. In order to prevent interoperability
                  problems it is recommended that systems sending I048/220 in an End
                  of Track Message continue to do so.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-220-001: Parse valid Aircraft Address
- TC-048-220-002: Verify format compliance
- TC-048-220-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-230: Parse Communications/ACAS Capability and Flight Status (I048/230)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/230 (Communications/ACAS Capability and Flight Status) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Communications capability of the transponder, capability of the on-board
            ACAS equipment and flight status.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Note:
                - This  item  shall  be  present  in  every  ASTERIX  record  conveying
                  data  related  to  a  Mode  S  target,  except  for  an  End  of  Track
                  Message (i.e. I048/170, First Extension, Bit 8 is set to 1) in which
                  this  Data  Item  is  optional.  If  the  datalink  capability  has  not  been
                  extracted yet, bits 16/14 shall be set to zero.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-230-001: Parse valid Communications/ACAS Capability and Flight Status
- TC-048-230-002: Verify format compliance
- TC-048-230-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-240: Parse Aircraft Identification (I048/240)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/240 (Aircraft Identification) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Aircraft identification (in 8 characters) obtained from an aircraft
            equipped with a Mode S transponder.
        

**Implementation Notes:**
- Format: Fixed (6 bytes)
- Rule: optional
- Note: 
            Notes:

                1. This data item contains the flight identification as available in
                   the respective Mode S transponder registers.
                2. The  Encoding  Rule  for  Data  Item  I048/240  has  been  relaxed  in  Edition
                   1.30 for the End of Track Message. In order to prevent interoperability
                   problems it is recommended that systems sending I048/240 in an End of
                   Track Message continue to do so.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-240-001: Parse valid Aircraft Identification
- TC-048-240-002: Verify format compliance
- TC-048-240-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-250: Parse BDS Register Data (I048/250)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/250 (BDS Register Data) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            BDS Register Data as extracted from the aircraft transponder.
        

**Implementation Notes:**
- Format: Repetitive
- Rule: optional
- Note: 
            Notes:

                1. For the transmission of BDS Register 2,0, Data Item I048/240 is used.
                2. For the transmission of BDS Register 3,0, Data Item I048/260 is used. In
                   case of ACAS Xu (as defined in [3]), the Resolution Advisory consists of two
                   parts (BDS Register 3,0 and BDS Register 3,1). BDS Register 3,1 will be
                   transmitted using Data Item I048/250. For the detailed definition of BDS
                   Register 3,0 and 3,1please refer to [2] Tables B-3-48a and B-3-49.
                3. In case of data extracted via Comm-B broadcast, all bits of fields BDS1 and
                   BDS2 are set to 0; in case of data extracted via GICB requests, the fields
                   BDS1 and BDS2 correspond to the GICB register number.
                4. The Encoding Rule for Data Item I048/250 has been relaxed in Edition 1.30
                   for the End of Track Message. In order to prevent interoperability problems
                   it is recommended that systems sending I048/250 in an End of Track
                   Message continue to do so.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-250-001: Parse valid BDS Register Data
- TC-048-250-002: Verify format compliance
- TC-048-250-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-260: Parse ACAS Resolution Advisory Report (I048/260)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/260 (ACAS Resolution Advisory Report) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Currently active Resolution Advisory (RA), if any, generated by the ACAS
            associated with the transponder transmitting the report and threat identity data.
        

**Implementation Notes:**
- Format: Fixed (7 bytes)
- Rule: optional
- Note: 
            Notes:

                1. Refer to ICAO Draft SARPs for ACAS for detailed explanations.
                2. In case of ACAS Xu, the Resolution Advisory consists of two parts (BDS30
                   and BDS31). BDS31 will be transmitted using item 250.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-260-001: Parse valid ACAS Resolution Advisory Report
- TC-048-260-002: Verify format compliance
- TC-048-260-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-RE: Parse Reserved Expansion Field (I048/RE)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/RE (Reserved Expansion Field) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Expansion
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-RE-001: Parse valid Reserved Expansion Field
- TC-048-RE-002: Verify format compliance
- TC-048-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

### REQ-LLR-048-SP: Parse Special Purpose Field (I048/SP)

**Parent:** REQ-HLR-CAT-048
**Category:** Functional
**Priority:** High

**Description:**
The parser shall extract Data Item I048/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT048 v1.30 specification.

**Data Item Definition:**

            Special Purpose Field
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-SP-001: Parse valid Special Purpose Field
- TC-048-SP-002: Verify format compliance
- TC-048-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat048_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Medium

---

## 3. Error Handling Requirements

### REQ-LLR-048-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT048 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-ERR-001: Truncated data items
- TC-048-ERR-002: Invalid repetition counts
- TC-048-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-048-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT048 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-048-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-048-010 | I048/010 | Data Source Identifier | High | Medium |
| REQ-LLR-048-020 | I048/020 | Target Report Descriptor | High | Medium |
| REQ-LLR-048-030 | I048/030 | Warning/Error Conditions and Target Clas | High | Medium |
| REQ-LLR-048-040 | I048/040 | Measured Position in Polar Co-ordinates | High | Medium |
| REQ-LLR-048-042 | I048/042 | Calculated Position in Cartesian Co-ordi | High | Medium |
| REQ-LLR-048-050 | I048/050 | Mode-2 Code in Octal Representation | High | Medium |
| REQ-LLR-048-055 | I048/055 | Mode-1 Code in Octal Representation | High | Medium |
| REQ-LLR-048-060 | I048/060 | Mode-2 Code Confidence Indicator | High | Medium |
| REQ-LLR-048-065 | I048/065 | Mode-1 Code Confidence Indicator | High | Medium |
| REQ-LLR-048-070 | I048/070 | Mode-3/A Code in Octal Representation | High | Medium |
| REQ-LLR-048-080 | I048/080 | Mode-3/A Code Confidence Indicator | High | Medium |
| REQ-LLR-048-090 | I048/090 | Flight Level in Binary Representation | High | Medium |
| REQ-LLR-048-100 | I048/100 | Mode-C Code and Code Confidence Indicato | High | Medium |
| REQ-LLR-048-110 | I048/110 | Height Measured by a 3D Radar | High | Medium |
| REQ-LLR-048-120 | I048/120 | Radial Doppler Speed | High | Medium |
| REQ-LLR-048-130 | I048/130 | Radar Plot Characteristics | High | Medium |
| REQ-LLR-048-140 | I048/140 | Time of Day | High | Medium |
| REQ-LLR-048-161 | I048/161 | Track Number | High | Medium |
| REQ-LLR-048-170 | I048/170 | Track Status | High | Medium |
| REQ-LLR-048-200 | I048/200 | Calculated Track Velocity in Polar Co-or | High | Medium |
| REQ-LLR-048-210 | I048/210 | Track Quality | High | Medium |
| REQ-LLR-048-220 | I048/220 | Aircraft Address | High | Medium |
| REQ-LLR-048-230 | I048/230 | Communications/ACAS Capability and Fligh | High | Medium |
| REQ-LLR-048-240 | I048/240 | Aircraft Identification | High | Medium |
| REQ-LLR-048-250 | I048/250 | BDS Register Data | High | Medium |
| REQ-LLR-048-260 | I048/260 | ACAS Resolution Advisory Report | High | Medium |
| REQ-LLR-048-RE | I048/RE | Reserved Expansion Field | High | Medium |
| REQ-LLR-048-SP | I048/SP | Special Purpose Field | High | Medium |
| REQ-LLR-048-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-048-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 30

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-048
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-048-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT048 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
