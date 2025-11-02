# Low-Level Requirements - ASTERIX Category 240
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT240-001
**Revision:** 1.0
**Date:** 2025-11-02
**Parent HLR:** REQ-HLR-CAT-240
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category 240 (Radar Video Transmission) v1.3.

**Parent Requirement:** REQ-HLR-CAT-240 - Parse ASTERIX Category 240

Each data item in CAT240 has corresponding LLRs defining parsing behavior.

---

## 2. CAT240 Data Item Requirements

### REQ-LLR-240-000: Parse Message Type (I240/000)

**Parent:** REQ-HLR-CAT-240
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I240/000 (Message Type) as specified in the EUROCONTROL ASTERIX CAT240 v1.3 specification.

**Data Item Definition:**
This Data Item allows for a more convenient handling of the messages at the receiver side by further defining the type of transaction.

**Implementation Notes:**
- Format: Fixed (1 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-000-001: Parse valid Message Type
- TC-240-000-002: Verify format compliance
- TC-240-000-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat240_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-240-010: Parse Data Source Identifier (I240/010)

**Parent:** REQ-HLR-CAT-240
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I240/010 (Data Source Identifier) as specified in the EUROCONTROL ASTERIX CAT240 v1.3 specification.

**Data Item Definition:**
Identification of the system from which the data are received.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-010-001: Parse valid Data Source Identifier
- TC-240-010-002: Verify format compliance
- TC-240-010-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat240_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-240-020: Parse Video Record Header (I240/020)

**Parent:** REQ-HLR-CAT-240
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I240/020 (Video Record Header) as specified in the EUROCONTROL ASTERIX CAT240 v1.3 specification.

**Data Item Definition:**
Contains a message sequence identifier.

**Implementation Notes:**
- Format: Fixed (4 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-020-001: Parse valid Video Record Header
- TC-240-020-002: Verify format compliance
- TC-240-020-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat240_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-240-030: Parse Video Summary (I240/030)

**Parent:** REQ-HLR-CAT-240
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I240/030 (Video Summary) as specified in the EUROCONTROL ASTERIX CAT240 v1.3 specification.

**Data Item Definition:**
Contains an ASCII string (free text to define stream meta data).

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-030-001: Parse valid Video Summary
- TC-240-030-002: Verify format compliance
- TC-240-030-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat240_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-240-040: Parse Video Header Nano (I240/040)

**Parent:** REQ-HLR-CAT-240
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I240/040 (Video Header Nano) as specified in the EUROCONTROL ASTERIX CAT240 v1.3 specification.

**Data Item Definition:**
Defines a group of video cells corresponding to a video radial: all cells have the same size in azimuth and range and are consecutive in range.

**Implementation Notes:**
- Format: Fixed (12 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-040-001: Parse valid Video Header Nano
- TC-240-040-002: Verify format compliance
- TC-240-040-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat240_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-240-041: Parse Video Header Femto (I240/041)

**Parent:** REQ-HLR-CAT-240
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I240/041 (Video Header Femto) as specified in the EUROCONTROL ASTERIX CAT240 v1.3 specification.

**Data Item Definition:**
Defines a group of video cells corresponding to a video radial: all cells have the same size in azimuth and range and are consecutive in range.

**Implementation Notes:**
- Format: Fixed (12 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-041-001: Parse valid Video Header Femto
- TC-240-041-002: Verify format compliance
- TC-240-041-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat240_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-240-048: Parse Video Cells Resolution and Data Compression Indicator (I240/048)

**Parent:** REQ-HLR-CAT-240
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I240/048 (Video Cells Resolution and Data Compression Indicator) as specified in the EUROCONTROL ASTERIX CAT240 v1.3 specification.

**Data Item Definition:**
This Data Item defines the bit resolution used in the coding of the video signal amplitude in all cells of the video group as well as an indicator whether data compression has been applied.

**Implementation Notes:**
- Format: Fixed (2 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-048-001: Parse valid Video Cells Resolution and Data Compression Indicator
- TC-240-048-002: Verify format compliance
- TC-240-048-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat240_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-240-049: Parse Video Octets and Video Cells Counters (I240/049)

**Parent:** REQ-HLR-CAT-240
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I240/049 (Video Octets and Video Cells Counters) as specified in the EUROCONTROL ASTERIX CAT240 v1.3 specification.

**Data Item Definition:**
This Data Item contains the number of valid octets (i.e. nonempty octets) used in the coding of the video signal amplitude and the number of valid cells in the video group

**Implementation Notes:**
- Format: Fixed (5 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-049-001: Parse valid Video Octets and Video Cells Counters
- TC-240-049-002: Verify format compliance
- TC-240-049-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat240_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-240-050: Parse Video Block Low Data Volume (I240/050)

**Parent:** REQ-HLR-CAT-240
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I240/050 (Video Block Low Data Volume) as specified in the EUROCONTROL ASTERIX CAT240 v1.3 specification.

**Data Item Definition:**
Contains a group of video cells corresponding to a video radial; all cells have the same size in azimuth and range and are consecutive in range. This item shall be used in cases where a low data volume, up to 1020 bytes, will be transmitted.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-050-001: Parse valid Video Block Low Data Volume
- TC-240-050-002: Verify format compliance
- TC-240-050-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat240_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-240-051: Parse Video Block Medium Data Volume (I240/051)

**Parent:** REQ-HLR-CAT-240
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I240/051 (Video Block Medium Data Volume) as specified in the EUROCONTROL ASTERIX CAT240 v1.3 specification.

**Data Item Definition:**
Contains a group of video cells corresponding to a video radial; all cells have the same size in azimuth and range and are consecutive in range. This item shall be used in cases where a medium data volume, up to 16320 bytes, will be transmitted.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-051-001: Parse valid Video Block Medium Data Volume
- TC-240-051-002: Verify format compliance
- TC-240-051-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat240_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-240-052: Parse Video Block High Data Volume (I240/052)

**Parent:** REQ-HLR-CAT-240
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I240/052 (Video Block High Data Volume) as specified in the EUROCONTROL ASTERIX CAT240 v1.3 specification.

**Data Item Definition:**
Contains a group of video cells corresponding to a video radial; all cells have the same size in azimuth and range and are consecutive in range. This item shall be used in cases where a high data volume, up to 65024 bytes, will be transmitted.

**Implementation Notes:**
- Format: Repetitive
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-052-001: Parse valid Video Block High Data Volume
- TC-240-052-002: Verify format compliance
- TC-240-052-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat240_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-240-140: Parse Time Of Day (I240/140)

**Parent:** REQ-HLR-CAT-240
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I240/140 (Time Of Day) as specified in the EUROCONTROL ASTERIX CAT240 v1.3 specification.

**Data Item Definition:**
Absolute time stamping expressed as UTC.

**Implementation Notes:**
- Format: Fixed (3 bytes)
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-140-001: Parse valid Time Of Day
- TC-240-140-002: Verify format compliance
- TC-240-140-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat240_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-240-SP: Parse Special Purpose (I240/SP)

**Parent:** REQ-HLR-CAT-240
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I240/SP (Special Purpose) as specified in the EUROCONTROL ASTERIX CAT240 v1.3 specification.

**Data Item Definition:**
Special information

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-SP-001: Parse valid Special Purpose
- TC-240-SP-002: Verify format compliance
- TC-240-SP-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat240_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

### REQ-LLR-240-RE: Parse Reserved Expansion Field (I240/RE)

**Parent:** REQ-HLR-CAT-240
**Category:** Functional
**Priority:** Medium

**Description:**
The parser shall extract Data Item I240/RE (Reserved Expansion Field) as specified in the EUROCONTROL ASTERIX CAT240 v1.3 specification.

**Data Item Definition:**
RE

**Implementation Notes:**
- Format: Explicit
- Rule: optional

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-RE-001: Parse valid Reserved Expansion Field
- TC-240-RE-002: Verify format compliance
- TC-240-RE-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat240_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** Low

---

## 3. Error Handling Requirements

### REQ-LLR-240-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT240 data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-ERR-001: Truncated data items
- TC-240-ERR-002: Invalid repetition counts
- TC-240-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-240-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT240 data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-240-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
| REQ-LLR-240-000 | I240/000 | Message Type | Medium | Low |
| REQ-LLR-240-010 | I240/010 | Data Source Identifier | Medium | Low |
| REQ-LLR-240-020 | I240/020 | Video Record Header | Medium | Low |
| REQ-LLR-240-030 | I240/030 | Video Summary | Medium | Low |
| REQ-LLR-240-040 | I240/040 | Video Header Nano | Medium | Low |
| REQ-LLR-240-041 | I240/041 | Video Header Femto | Medium | Low |
| REQ-LLR-240-048 | I240/048 | Video Cells Resolution and Data Compress | Medium | Low |
| REQ-LLR-240-049 | I240/049 | Video Octets and Video Cells Counters | Medium | Low |
| REQ-LLR-240-050 | I240/050 | Video Block Low Data Volume | Medium | Low |
| REQ-LLR-240-051 | I240/051 | Video Block Medium Data Volume | Medium | Low |
| REQ-LLR-240-052 | I240/052 | Video Block High Data Volume | Medium | Low |
| REQ-LLR-240-140 | I240/140 | Time Of Day | Medium | Low |
| REQ-LLR-240-SP | I240/SP | Special Purpose | Medium | Low |
| REQ-LLR-240-RE | I240/RE | Reserved Expansion Field | Medium | Low |
| REQ-LLR-240-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-240-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** 16

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-240
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-240-xxx-xxx)

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
| 1.0 | 2025-11-02 | Auto-generated | Initial LLR for CAT240 data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
