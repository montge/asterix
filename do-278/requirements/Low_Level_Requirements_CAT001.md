# Low-Level Requirements - ASTERIX Category 001
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT001-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-001
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 001 (Transmission of Monoradar Data Target Reports) v1.4.

**Parent Requirement:** REQ-HLR-CAT-001 - Parse ASTERIX Category 001

Each data item in CAT001 has corresponding LLRs defining parsing behavior.

---

## 2. CAT001 Data Item Requirements

### REQ-LLR-001-010: Parse Data Source Identifier (I001/010)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Identification of the radar station from which the data are received.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Note:
                The defined SACs are on the EUROCONTROL ASTERIX website
                (www.eurocontrol.int/asterix)
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-010-001: Parse valid Data Source Identifier
- TC-001-010-002: Verify format compliance
- TC-001-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-020: Parse Target Report Descriptor (I001/020)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/020 (Target Report Descriptor) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Type and characteristics of the radar data as transmitted by a radar station.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            Note:

                - Bit-7 (SIM) is used to identify a simulated target report as produced by a traffic simulator.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-020-001: Parse valid Target Report Descriptor
- TC-001-020-002: Verify format compliance
- TC-001-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-030: Parse Warning/Error Conditions (I001/030)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/030 (Warning/Error Conditions) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Warning/error conditions detected by a radar station for the target
            report involved.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            Notes:

                1. Warning/error condition values 0-63 are reserved for common
                   standard use, whereas the values 64-127 are application dependent.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-030-001: Parse valid Warning/Error Conditions
- TC-001-030-002: Verify format compliance
- TC-001-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-040: Parse Measured Position in Polar Co-ordinates (I001/040)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/040 (Measured Position in Polar Co-ordinates) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Measured position of an aircraft in local polar co-ordinates.
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: 
            Note:

                - When expressed in 16 bits, signed or unsigned azimuths have the same value.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-040-001: Parse valid Measured Position in Polar Co-ordinates
- TC-001-040-002: Verify format compliance
- TC-001-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-042: Parse Calculated Position in Cartesian Co-ordinates (I001/042)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/042 (Calculated Position in Cartesian Co-ordinates) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Calculated position of an aircraft in Cartesian co-ordinates.
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: 
            Notes:

                1. LSB is calculated as :math:`2^{-6+f}`.
                2. A default quantisation unit of 1/64 NM is obtained for a value of f = 0.
                3. Negative values are expressed in 2's complement form, bit-32 and
                   bit-16 shall be set to 0 for positive values and 1 for negative values.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-042-001: Parse valid Calculated Position in Cartesian Co-ordinates
- TC-001-042-002: Verify format compliance
- TC-001-042-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-050: Parse Mode-2 Code in Octal Representation (I001/050)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/050 (Mode-2 Code in Octal Representation) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Reply to Mode-2 interrogation.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Notes:

                1. Smoothed Mode-2 data (bit-14 set to one) is used when the plot
                   contains no Mode-2 code or the Mode-2 codes of the plot and track
                   are different.
                2. Bits-16/15 have no meaning in the case of a smoothed Mode-2 and
                   are set to 0 for a calculated track.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-050-001: Parse valid Mode-2 Code in Octal Representation
- TC-001-050-002: Verify format compliance
- TC-001-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-060: Parse Mode-2 Code Confidence Indicator (I001/060)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/060 (Mode-2 Code Confidence Indicator) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Confidence level for each bit of a Mode-2 reply as provided by a monopulse SSR station.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Note:
                - This Data Item is only transmitted if at least one pulse is of low quality.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-060-001: Parse valid Mode-2 Code Confidence Indicator
- TC-001-060-002: Verify format compliance
- TC-001-060-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-070: Parse Mode-3/A Code in Octal Representation (I001/070)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/070 (Mode-3/A Code in Octal Representation) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Mode-3/A code converted into octal representation.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Notes:

                1. The detector signals a garbled code (bit-15 set to one) when at
                   least two replies are overlapping.
                2. Smoothed Mode-3/A data (bit-14 set to a one) are used in the case
                   of the absence of Mode-3/A code information in the plot, or in the
                   case of a difference between the plot and track Mode-3/A code information.
                3. Bits-16/15 have no meaning in the case of a smoothed Mode-3/A and
                   are set to 0 for a calculated track.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-070-001: Parse valid Mode-3/A Code in Octal Representation
- TC-001-070-002: Verify format compliance
- TC-001-070-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-080: Parse Mode-3/A Code Confidence Indicator (I001/080)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/080 (Mode-3/A Code Confidence Indicator) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Confidence level for each bit of a Mode-3/A reply as provided by a monopulse SSR station.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-080-001: Parse valid Mode-3/A Code Confidence Indicator
- TC-001-080-002: Verify format compliance
- TC-001-080-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-090: Parse Mode-C Code in Binary Representation (I001/090)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/090 (Mode-C Code in Binary Representation) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Mode-C height converted into binary representation.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Notes:

                1. The detector signals a garbled code when at least two replies are overlapping.
                2. The maximum height which can be represented is 204 775 ft.
                   Practically the maximum valid value is 126 750 ft (refer to ICAO Annex 10).
                3. Negative values are expressed in 2's complement form, bit-14 is
                   set to 0 for positive values and 1 for negative values.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-090-001: Parse valid Mode-C Code in Binary Representation
- TC-001-090-002: Verify format compliance
- TC-001-090-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-100: Parse Mode-C Code and Code Confidence Indicator (I001/100)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/100 (Mode-C Code and Code Confidence Indicator) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Mode-C height in Gray notation as received from the transponder together
            with the confidence level for each reply bit as provided by a monopulse SSR station.
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional
- Note: 
            Notes:

                1. This Data Item is only transmitted if at least one pulse is of low quality.
                2. The detector signals a garbled code when at least two replies are overlapping.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-100-001: Parse valid Mode-C Code and Code Confidence Indicator
- TC-001-100-002: Verify format compliance
- TC-001-100-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-120: Parse Measured Radial Doppler Speed (I001/120)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/120 (Measured Radial Doppler Speed) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Radial component of the ground speed as measured by means of Doppler filter banks in radar signal processors.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
            Notes:
                1. LSB is calculated as :math:`2^{-14+f}`.
                2. A default quantisation unit of 14.0625 kt and a maximum of +/- 1 800 kt
                   is obtained for a value of f = 6.
                3. Negative values are expressed in 2's complement form, bit-8 is
                   set to 0 for positive values and 1 for negative values.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-120-001: Parse valid Measured Radial Doppler Speed
- TC-001-120-002: Verify format compliance
- TC-001-120-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-130: Parse Radar Plot Characteristics (I001/130)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/130 (Radar Plot Characteristics) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Additional information on the quality of the target report.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            Notes:
                - The actual meaning of the bits is application dependent.&quot;

                - This Data Item may contain parameters such as plot runlength
                  (primary and secondary), difference between primary and secondary
                  derived azimuth, pulse amplitude, etc.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-130-001: Parse valid Radar Plot Characteristics
- TC-001-130-002: Verify format compliance
- TC-001-130-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-131: Parse Received Power (I001/131)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/131 (Received Power) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Measurement of the received power.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
            Notes:

                1. POWER is the measured value of the power received on the sum
                   pattern for a plot.
                2. Negative values are expressed in 2's complement form, bit-8 is
                   set to 0 for positive values and 1 for negative values.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-131-001: Parse valid Received Power
- TC-001-131-002: Verify format compliance
- TC-001-131-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-141: Parse Truncated Time of Day (I001/141)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/141 (Truncated Time of Day) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Absolute time stamping expressed as Coordinated Universal Time (UTC) time.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Notes:

                1. The exchange of this Data Item allows the easy derivation of the
                   correct UTC time value, provided that the clocks at the data source
                   and sink(s) are less than 512 seconds out of synchronisation.
                   Special care has to be taken at the transition of an &quot;all ones&quot;
                   value to an &quot;all zeros&quot; value (every 512 seconds).
                2. The time of day value is reset to 0 each day at midnight.
                3. For time management in radar transmission applications, refer to
                   Part 1, paragraph 5.4 [Ref. 2].
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-141-001: Parse valid Truncated Time of Day
- TC-001-141-002: Verify format compliance
- TC-001-141-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-150: Parse Presence of X-Pulse (I001/150)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/150 (Presence of X-Pulse) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Presence of the X-Pulse for the various modes applied in the interrogation interlace pattern.
        

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional
- Note: 
            Note:

                - This Data Item is transmitted only if at least one X-pulse has been
                  received in a Mode-A, Mode-2 or Mode-C reply.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-150-001: Parse valid Presence of X-Pulse
- TC-001-150-002: Verify format compliance
- TC-001-150-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-161: Parse Track Plot Number (I001/161)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/161 (Track Plot Number) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            An integer value representing a unique reference to a track/plot record within a particular track/plot file.
        

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional
- Note: 
            Note:

                - The differentiation between track and plot number is either implicit
                  or is made via the Target Report Descriptor (Data Item I001/020).
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-161-001: Parse valid Track Plot Number
- TC-001-161-002: Verify format compliance
- TC-001-161-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-170: Parse Track Status (I001/170)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/170 (Track Status) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Status of track derived either from primary and/or secondary radar information.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            Notes:

                1. Bit-2 (GHO) is used to signal that the track is suspected to have
                   been generated by a fake target.
                2. Bit-4 (RDPC) is used to signal the discontinuity of the track numbers.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-170-001: Parse valid Track Status
- TC-001-170-002: Verify format compliance
- TC-001-170-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-200: Parse Calculated Track Velocity in Polar Co-ordinates (I001/200)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/200 (Calculated Track Velocity in Polar Co-ordinates) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Calculated track velocity expressed in polar co-ordinates.
        

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-200-001: Parse valid Calculated Track Velocity in Polar Co-ordinates
- TC-001-200-002: Verify format compliance
- TC-001-200-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-210: Parse Track Quality (I001/210)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/210 (Track Quality) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Relative track quality.
        

**Implementation Notes:**
- Format: Variable
- Rule: optional
- Note: 
            Note:

                - Actual bit signification is application dependent.
        

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-210-001: Parse valid Track Quality
- TC-001-210-002: Verify format compliance
- TC-001-210-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-SP: Parse Special Purpose Field (I001/SP)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/SP (Special Purpose Field) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Special Purpose Field
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-SP-001: Parse valid Special Purpose Field
- TC-001-SP-002: Verify format compliance
- TC-001-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-001-RFS: Parse Random Field Sequencing (I001/RFS)

**Parent:** REQ-HLR-CAT-001
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I001/RFS (Random Field Sequencing) as specified in the EUROCONTROL ASTERIX CAT001 v1.4 specification.

**Data Item Definition:**

            Random Field Sequencing
        

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-RFS-001: Parse valid Random Field Sequencing
- TC-001-RFS-002: Verify format compliance
- TC-001-RFS-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat001_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-001-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT001 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-ERR-001: Truncated data items
- TC-001-ERR-002: Invalid repetition counts
- TC-001-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-001-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT001 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-001-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-001-010 | I001/010 | Data Source Identifier | Medium | Low |
| REQ-LLR-001-020 | I001/020 | Target Report Descriptor | Medium | Low |
| REQ-LLR-001-030 | I001/030 | Warning/Error Conditions | Medium | Low |
| REQ-LLR-001-040 | I001/040 | Measured Position in Polar Co-ordinates | Medium | Low |
| REQ-LLR-001-042 | I001/042 | Calculated Position in Cartesian Co-ordi | Medium | Low |
| REQ-LLR-001-050 | I001/050 | Mode-2 Code in Octal Representation | Medium | Low |
| REQ-LLR-001-060 | I001/060 | Mode-2 Code Confidence Indicator | Medium | Low |
| REQ-LLR-001-070 | I001/070 | Mode-3/A Code in Octal Representation | Medium | Low |
| REQ-LLR-001-080 | I001/080 | Mode-3/A Code Confidence Indicator | Medium | Low |
| REQ-LLR-001-090 | I001/090 | Mode-C Code in Binary Representation | Medium | Low |
| REQ-LLR-001-100 | I001/100 | Mode-C Code and Code Confidence Indicato | Medium | Low |
| REQ-LLR-001-120 | I001/120 | Measured Radial Doppler Speed | Medium | Low |
| REQ-LLR-001-130 | I001/130 | Radar Plot Characteristics | Medium | Low |
| REQ-LLR-001-131 | I001/131 | Received Power | Medium | Low |
| REQ-LLR-001-141 | I001/141 | Truncated Time of Day | Medium | Low |
| REQ-LLR-001-150 | I001/150 | Presence of X-Pulse | Medium | Low |
| REQ-LLR-001-161 | I001/161 | Track Plot Number | Medium | Low |
| REQ-LLR-001-170 | I001/170 | Track Status | Medium | Low |
| REQ-LLR-001-200 | I001/200 | Calculated Track Velocity in Polar Co-or | Medium | Low |
| REQ-LLR-001-210 | I001/210 | Track Quality | Medium | Low |
| REQ-LLR-001-SP | I001/SP | Special Purpose Field | Medium | Low |
| REQ-LLR-001-RFS | I001/RFS | Random Field Sequencing | Medium | Low |
| REQ-LLR-001-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-001-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 24

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-001
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-001-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT001 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
