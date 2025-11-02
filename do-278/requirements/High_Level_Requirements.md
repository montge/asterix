# High-Level Requirements (HLR)
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** HLR-ASTERIX-001
**Revision:** 2.0
**Date:** 2025-10-17
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

### 1.1 Purpose

This document defines the high-level requirements for the ASTERIX Decoder software. These requirements describe WHAT the software shall do from a system and functional perspective.

### 1.2 Scope

Requirements cover:
- ASTERIX protocol parsing (Categories 001-252)
- Input/output handling
- Data validation
- Error handling
- Python and C++ APIs
- Configuration management

### 1.3 Organization

Requirements are organized by:
- **REQ-HLR-SYS-xxx**: System-level requirements
- **REQ-HLR-CAT-xxx**: Category-specific parsing requirements
- **REQ-HLR-IO-xxx**: Input/output requirements
- **REQ-HLR-API-xxx**: API requirements
- **REQ-HLR-CFG-xxx**: Configuration requirements
- **REQ-HLR-ERR-xxx**: Error handling requirements

---

## 2. System-Level Requirements

### REQ-HLR-SYS-001: ASTERIX Protocol Compliance

**Category:** Functional
**Priority:** Critical
**Source:** EUROCONTROL ASTERIX Specifications

**Description:**
The software shall decode and parse ASTERIX binary data in accordance with EUROCONTROL ASTERIX specifications.

**Rationale:**
Core functionality - the software must correctly interpret ASTERIX protocol data for ATM surveillance systems.

**Derived Requirements:**
- REQ-HLR-CAT-048 (CAT048 parsing)
- REQ-HLR-CAT-062 (CAT062 parsing)
- [Additional category requirements...]

**Verification Method:** Test
**Verification Criteria:**
- Parse sample ASTERIX data correctly
- All data items extracted accurately
- Values match reference implementation
- CRC validation passes

**Safety Impact:** High
**DO-278A Traceability:** Objective 1.1 (High-level requirements)

---

### REQ-HLR-SYS-002: Data Integrity Verification

**Category:** Safety
**Priority:** Critical
**Source:** ATM Safety Requirements

**Description:**
The software shall validate data integrity of ASTERIX messages using CRC checks and length validation.

**Rationale:**
Corrupted surveillance data could lead to incorrect track information, impacting controller decision-making.

**Derived Requirements:**
- REQ-LLR-VAL-001 (CRC calculation)
- REQ-LLR-VAL-002 (Length validation)

**Verification Method:** Test
**Verification Criteria:**
- Detect corrupted messages
- Reject invalid data
- Report data integrity errors

**Safety Impact:** High
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-SYS-003: Multiple ASTERIX Categories

**Category:** Functional
**Priority:** High
**Source:** User Requirements

**Description:**
The software shall support parsing of multiple ASTERIX categories as defined by EUROCONTROL, including but not limited to: CAT001, CAT002, CAT004, CAT010, CAT011, CAT019, CAT020, CAT021, CAT023, CAT025, CAT034, CAT048, CAT062, CAT065, CAT240, CAT252.

**Rationale:**
Different radar and surveillance systems use different ASTERIX categories.

**Derived Requirements:**
- One REQ-HLR-CAT-xxx per supported category

**Verification Method:** Test
**Verification Criteria:**
- Successfully parse data from each supported category
- Correct identification of category number
- Accurate extraction of category-specific data items

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

## 3. Category-Specific Requirements

### REQ-HLR-CAT-048: Parse ASTERIX Category 048

**Category:** Functional
**Priority:** Critical
**Source:** EUROCONTROL ASTERIX CAT048 v1.21 Specification

**Description:**
The software shall parse ASTERIX Category 048 (Transmission of Monoradar Target Reports) data blocks according to EUROCONTROL specification CAT048 v1.21, extracting all defined data items.

**Rationale:**
Category 048 is a fundamental monoradar surveillance category used in ATC systems for primary and secondary radar target reports. Correct parsing is critical for accurate radar track display.

**Data Items to Parse:**
- I048/010: Data Source Identifier (SAC/SIC)
- I048/020: Target Report Descriptor
- I048/030: Warning/Error Conditions
- I048/040: Measured Position in Polar Coordinates
- I048/042: Calculated Position in Cartesian Coordinates
- I048/050: Mode-2 Code
- I048/055: Mode-1 Code
- I048/060: Mode-2 Code Confidence Indicator
- I048/065: Mode-1 Code Confidence Indicator
- I048/070: Mode-3/A Code
- I048/080: Mode-3/A Code Confidence Indicator
- I048/090: Flight Level
- I048/100: Mode-C Code and Code Confidence Indicator
- I048/110: Height Measured by a 3D Radar
- I048/120: Radial Doppler Speed
- I048/130: Radar Plot Characteristics
- I048/140: Time Of Day
- I048/161: Track Number
- I048/170: Track Status
- I048/200: Calculated Track Velocity in Polar Coordinates
- I048/210: Track Quality
- I048/220: Aircraft Address
- I048/230: Communications Capability
- I048/240: Aircraft Identification
- I048/250: Mode S Comm B Data
- I048/260: ACAS Resolution Advisory Report
- I048/SP: Special Purpose Field

**Derived Requirements:**
- REQ-LLR-048-010: Parse Data Source Identifier
- REQ-LLR-048-020: Parse Target Report Descriptor
- REQ-LLR-048-040: Parse Measured Position
- REQ-LLR-048-070: Parse Mode-3/A Code
- REQ-LLR-048-140: Parse Time Of Day
- REQ-LLR-048-240: Parse Aircraft Identification
- [One LLR per data item - see Low_Level_Requirements.md]

**Verification Method:** Test
**Verification Criteria:**
- All CAT048 data items correctly extracted
- Values match EUROCONTROL reference data
- Test with sample data: `install/sample_data/cat_034_048.pcap`
- Output matches expected: `install/sample_output/cat_034_048.txt`

**Safety Impact:** High
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-062: Parse ASTERIX Category 062

**Category:** Functional
**Priority:** Critical
**Source:** EUROCONTROL ASTERIX CAT062 v1.18 Specification

**Description:**
The software shall parse ASTERIX Category 062 (System Track Data) data blocks according to EUROCONTROL specification CAT062 v1.18, extracting all defined data items.

**Rationale:**
Category 062 contains processed track data from tracker systems, essential for ATC display systems.

**Derived Requirements:**
- REQ-LLR-062-xxx: [To be documented - similar structure to CAT048]

**Verification Method:** Test
**Verification Criteria:**
- All CAT062 data items correctly extracted
- Test with sample data: `install/sample_data/cat_062_065.pcap`

**Safety Impact:** High
**DO-278A Traceability:** Objective 1.1

---

## 3. Category-Specific Requirements

### REQ-HLR-CAT-001: Parse ASTERIX Category 001

**Category:** Functional
**Priority:** High
**Source:** EUROCONTROL ASTERIX CAT001 Specification

**Description:**
The software shall parse ASTERIX Category 001 (Radar Detection Messages) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 001 provides radar detection messages essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT001.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT001 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-002: Parse ASTERIX Category 002

**Category:** Functional
**Priority:** High
**Source:** EUROCONTROL ASTERIX CAT002 Specification

**Description:**
The software shall parse ASTERIX Category 002 (Monoradar Target Reports) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 002 provides monoradar target reports essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT002.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT002 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** High
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-004: Parse ASTERIX Category 004

**Category:** Functional
**Priority:** Critical
**Source:** EUROCONTROL ASTERIX CAT004 Specification

**Description:**
The software shall parse ASTERIX Category 004 (Safety Net Messages) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 004 provides safety net messages essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT004.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT004 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** High
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-008: Parse ASTERIX Category 008

**Category:** Functional
**Priority:** Medium
**Source:** EUROCONTROL ASTERIX CAT008 Specification

**Description:**
The software shall parse ASTERIX Category 008 (Monoradar Derived Weather Information) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 008 provides monoradar derived weather information essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT008.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT008 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-010: Parse ASTERIX Category 010

**Category:** Functional
**Priority:** Medium
**Source:** EUROCONTROL ASTERIX CAT010 Specification

**Description:**
The software shall parse ASTERIX Category 010 (Monoradar Service Messages) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 010 provides monoradar service messages essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT010.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT010 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-011: Parse ASTERIX Category 011

**Category:** Functional
**Priority:** High
**Source:** EUROCONTROL ASTERIX CAT011 Specification

**Description:**
The software shall parse ASTERIX Category 011 (Track Status Messages) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 011 provides track status messages essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT011.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT011 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-015: Parse ASTERIX Category 015

**Category:** Functional
**Priority:** Medium
**Source:** EUROCONTROL ASTERIX CAT015 Specification

**Description:**
The software shall parse ASTERIX Category 015 (Broadcast Messages) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 015 provides broadcast messages essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT015.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT015 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Low
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-019: Parse ASTERIX Category 019

**Category:** Functional
**Priority:** High
**Source:** EUROCONTROL ASTERIX CAT019 Specification

**Description:**
The software shall parse ASTERIX Category 019 (Multilateration System Status Messages) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 019 provides multilateration system status messages essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT019.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT019 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-020: Parse ASTERIX Category 020

**Category:** Functional
**Priority:** Critical
**Source:** EUROCONTROL ASTERIX CAT020 Specification

**Description:**
The software shall parse ASTERIX Category 020 (Multilateration Target Reports) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 020 provides multilateration target reports essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT020.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT020 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** High
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-021: Parse ASTERIX Category 021

**Category:** Functional
**Priority:** Critical
**Source:** EUROCONTROL ASTERIX CAT021 Specification

**Description:**
The software shall parse ASTERIX Category 021 (ADS-B Target Reports) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 021 provides ads-b target reports essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT021.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT021 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** High
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-023: Parse ASTERIX Category 023

**Category:** Functional
**Priority:** Medium
**Source:** EUROCONTROL ASTERIX CAT023 Specification

**Description:**
The software shall parse ASTERIX Category 023 (CNS/ATM Ground Station Status Reports) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 023 provides cns/atm ground station status reports essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT023.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT023 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-025: Parse ASTERIX Category 025

**Category:** Functional
**Priority:** Medium
**Source:** EUROCONTROL ASTERIX CAT025 Specification

**Description:**
The software shall parse ASTERIX Category 025 (CNS/ATM Ground System Status Reports) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 025 provides cns/atm ground system status reports essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT025.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT025 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-030: Parse ASTERIX Category 030

**Category:** Functional
**Priority:** High
**Source:** EUROCONTROL ASTERIX CAT030 Specification

**Description:**
The software shall parse ASTERIX Category 030 (ARTAS Service Messages) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 030 provides artas service messages essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT030.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT030 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-031: Parse ASTERIX Category 031

**Category:** Functional
**Priority:** High
**Source:** EUROCONTROL ASTERIX CAT031 Specification

**Description:**
The software shall parse ASTERIX Category 031 (ARTAS Track Status Messages) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 031 provides artas track status messages essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT031.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT031 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-032: Parse ASTERIX Category 032

**Category:** Functional
**Priority:** Low
**Source:** EUROCONTROL ASTERIX CAT032 Specification

**Description:**
The software shall parse ASTERIX Category 032 (Miniature Range Test Messages) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 032 provides miniature range test messages essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT032.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT032 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Low
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-034: Parse ASTERIX Category 034

**Category:** Functional
**Priority:** High
**Source:** EUROCONTROL ASTERIX CAT034 Specification

**Description:**
The software shall parse ASTERIX Category 034 (Monoradar Service Messages) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 034 provides monoradar service messages essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT034.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT034 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-048: Parse ASTERIX Category 048

**Category:** Functional
**Priority:** Critical
**Source:** EUROCONTROL ASTERIX CAT048 Specification

**Description:**
The software shall parse ASTERIX Category 048 (Transmission of Monoradar Target Reports) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 048 provides transmission of monoradar target reports essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT048.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT048 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** High
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-062: Parse ASTERIX Category 062

**Category:** Functional
**Priority:** Critical
**Source:** EUROCONTROL ASTERIX CAT062 Specification

**Description:**
The software shall parse ASTERIX Category 062 (System Track Data) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 062 provides system track data essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT062.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT062 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** High
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-063: Parse ASTERIX Category 063

**Category:** Functional
**Priority:** Medium
**Source:** EUROCONTROL ASTERIX CAT063 Specification

**Description:**
The software shall parse ASTERIX Category 063 (Sensor Status Messages) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 063 provides sensor status messages essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT063.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT063 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-065: Parse ASTERIX Category 065

**Category:** Functional
**Priority:** High
**Source:** EUROCONTROL ASTERIX CAT065 Specification

**Description:**
The software shall parse ASTERIX Category 065 (SDPS Service Status Messages) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 065 provides sdps service status messages essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT065.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT065 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-205: Parse ASTERIX Category 205

**Category:** Functional
**Priority:** Medium
**Source:** EUROCONTROL ASTERIX CAT205 Specification

**Description:**
The software shall parse ASTERIX Category 205 (Multilateration System Status - Extended) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 205 provides multilateration system status - extended essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT205.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT205 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-240: Parse ASTERIX Category 240

**Category:** Functional
**Priority:** Medium
**Source:** EUROCONTROL ASTERIX CAT240 Specification

**Description:**
The software shall parse ASTERIX Category 240 (Video Transmission) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 240 provides video transmission essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT240.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT240 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Low
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-247: Parse ASTERIX Category 247

**Category:** Functional
**Priority:** Low
**Source:** EUROCONTROL ASTERIX CAT247 Specification

**Description:**
The software shall parse ASTERIX Category 247 (Reserved for ADS-B Messages) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 247 provides reserved for ads-b messages essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT247.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT247 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** Low
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CAT-252: Parse ASTERIX Category 252

**Category:** Functional
**Priority:** High
**Source:** EUROCONTROL ASTERIX CAT252 Specification

**Description:**
The software shall parse ASTERIX Category 252 (ADS-B Transponder Tracks) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category 252 provides ads-b transponder tracks essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT252.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT252 data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** High
**DO-278A Traceability:** Objective 1.1

---


## 4. Input/Output Requirements

### REQ-HLR-IO-001: Multiple Input Formats

**Category:** Functional
**Priority:** High
**Source:** User Requirements

**Description:**
The software shall support reading ASTERIX data from multiple input formats:
1. Raw ASTERIX binary
2. PCAP files (tcpdump/Wireshark format)
3. HDLC-encapsulated data
4. FINAL packet format
5. GPS packet format
6. Network multicast streams
7. Standard input (stdin)

**Rationale:**
Different operational environments provide ASTERIX data in different formats.

**Derived Requirements:**
- REQ-LLR-IO-RAW: Parse raw ASTERIX binary
- REQ-LLR-IO-PCAP: Parse PCAP files
- REQ-LLR-IO-HDLC: Parse HDLC frames
- REQ-LLR-IO-FINAL: Parse FINAL packets
- REQ-LLR-IO-GPS: Parse GPS packets
- REQ-LLR-IO-MCAST: Receive multicast streams

**Verification Method:** Test
**Verification Criteria:**
- Successfully parse data from each input format
- Correct extraction of ASTERIX data from encapsulation

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-IO-002: Multiple Output Formats

**Category:** Functional
**Priority:** High
**Source:** User Requirements

**Description:**
The software shall support outputting parsed ASTERIX data in multiple formats:
1. Human-readable text
2. Line-delimited text (parsable)
3. JSON (compact, one object per line)
4. JSON (human-readable, pretty-printed)
5. JSON (extensive, with descriptions)
6. XML (compact, one object per line)
7. XML (human-readable, formatted)

**Rationale:**
Different downstream systems and use cases require different output formats.

**Derived Requirements:**
- REQ-LLR-OUT-TEXT: Generate text output
- REQ-LLR-OUT-JSON: Generate JSON output
- REQ-LLR-OUT-XML: Generate XML output

**Verification Method:** Test
**Verification Criteria:**
- Output format is valid (JSON/XML parsable)
- All parsed data present in output
- Format matches specification

**Safety Impact:** Low
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-IO-003: Data Filtering

**Category:** Functional
**Priority:** Medium
**Source:** User Requirements

**Description:**
The software shall support filtering output to include only specified data items based on a filter configuration file.

**Rationale:**
Users may only need specific data items, reducing output volume.

**Derived Requirements:**
- REQ-LLR-FLT-001: Load filter configuration
- REQ-LLR-FLT-002: Apply filter to output

**Verification Method:** Test
**Verification Criteria:**
- Filter correctly includes/excludes items
- Test with: `install/sample_output/filter.txt`

**Safety Impact:** Low
**DO-278A Traceability:** Objective 1.1

---

## 5. API Requirements

### REQ-HLR-API-001: Python API

**Category:** Interface
**Priority:** High
**Source:** User Requirements

**Description:**
The software shall provide a Python API for parsing ASTERIX data programmatically, including:
- `parse(data)` - Parse ASTERIX binary data
- `parse_with_offset(data, offset, blocks_count)` - Incremental parsing
- `describe(category, item, field, value)` - Get descriptions
- `init(filename)` - Load custom configuration

**Rationale:**
Python integration enables use in analysis tools, scripts, and applications.

**Derived Requirements:**
- REQ-LLR-API-PY-PARSE: Implement parse function
- REQ-LLR-API-PY-OFFSET: Implement incremental parsing
- REQ-LLR-API-PY-DESC: Implement description function

**Verification Method:** Test
**Verification Criteria:**
- Python functions return correct results
- API matches documentation
- Examples work correctly

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-API-002: C/C++ Library API

**Category:** Interface
**Priority:** High
**Source:** User Requirements

**Description:**
The software shall provide C/C++ library APIs for integration into other applications, available as both shared libraries (.so, .dll) and static libraries (.a, .lib).

**Rationale:**
Enables integration into C/C++ applications and systems.

**Derived Requirements:**
- REQ-LLR-API-CPP-PARSE: C++ parsing interface
- REQ-LLR-API-C-PARSE: C wrapper interface
- REQ-LLR-LIB-SHARED: Shared library build
- REQ-LLR-LIB-STATIC: Static library build

**Verification Method:** Test
**Verification Criteria:**
- Libraries link correctly
- API functions work as documented
- Thread-safe (if required)

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-API-003: Command-Line Interface

**Category:** Interface
**Priority:** High
**Source:** User Requirements

**Description:**
The software shall provide a command-line interface (CLI) executable with options for:
- Input source selection (-f file, -i multicast)
- Input format selection (-P pcap, -R oradis, -H hdlc, -G gps, -F final)
- Output format selection (-j json, -x xml, -l line, -jh jsonh, -xh xmlh, -je json-extensive)
- Filter configuration (-LF filter_file)
- Configuration file (-d config_file)
- Verbose mode (-v)
- Version display (-V)
- Help display (-h)

**Rationale:**
CLI enables standalone use for data analysis, debugging, and integration with shell scripts.

**Derived Requirements:**
- REQ-LLR-CLI-ARGS: Argument parsing
- REQ-LLR-CLI-EXEC: Execute requested operation

**Verification Method:** Test
**Verification Criteria:**
- All CLI options work correctly
- Help text is accurate
- Error messages are clear

**Safety Impact:** Low
**DO-278A Traceability:** Objective 1.1

---

## 6. Configuration Requirements

### REQ-HLR-CFG-001: XML Configuration Files

**Category:** Data
**Priority:** Critical
**Source:** Design Requirement

**Description:**
The software shall load ASTERIX category definitions from XML configuration files conforming to the `asterix.dtd` schema. Each category shall have a separate XML file defining its data items, formats, and User Application Profile (UAP).

**Rationale:**
XML configuration allows flexible definition of ASTERIX categories without code changes.

**Derived Requirements:**
- REQ-LLR-CFG-XML-LOAD: Load XML configuration
- REQ-LLR-CFG-XML-VALID: Validate against DTD
- REQ-LLR-CFG-UAP: Parse UAP definitions

**Verification Method:** Test
**Verification Criteria:**
- Valid XML files load successfully
- Invalid XML files rejected with error
- All categories defined in config are available

**Safety Impact:** High
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CFG-002: Default Configuration

**Category:** Data
**Priority:** High
**Source:** User Requirements

**Description:**
The software shall include default ASTERIX category configuration files for all supported categories, automatically loaded on initialization.

**Rationale:**
Users should not need to manually configure common categories.

**Derived Requirements:**
- REQ-LLR-CFG-DEFAULT: Include default configs in package

**Verification Method:** Test
**Verification Criteria:**
- Software works without user-provided config
- All standard categories available by default

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-CFG-003: Custom Configuration

**Category:** Data
**Priority:** Medium
**Source:** User Requirements

**Description:**
The software shall allow users to load custom ASTERIX category definitions via the `init()` function (Python) or configuration file parameter (CLI).

**Rationale:**
Enables support for custom ASTERIX categories or variations.

**Derived Requirements:**
- REQ-LLR-CFG-CUSTOM: Load user-specified config

**Verification Method:** Test
**Verification Criteria:**
- Custom config loads successfully
- Custom categories parse correctly

**Safety Impact:** Low
**DO-278A Traceability:** Objective 1.1

---

## 7. Error Handling Requirements

### REQ-HLR-ERR-001: Graceful Error Handling

**Category:** Safety
**Priority:** Critical
**Source:** Safety Requirements

**Description:**
The software shall handle all error conditions gracefully without crashing, providing meaningful error messages for:
- Invalid input data
- Corrupted ASTERIX messages
- Unknown categories
- Configuration errors
- I/O errors
- Memory allocation failures

**Rationale:**
Unexpected crashes could disrupt ATM operations. Errors must be detected and reported safely.

**Derived Requirements:**
- REQ-LLR-ERR-INVALID: Handle invalid input
- REQ-LLR-ERR-CORRUPT: Handle corrupted data
- REQ-LLR-ERR-UNKNOWN: Handle unknown categories
- REQ-LLR-ERR-CONFIG: Handle config errors
- REQ-LLR-ERR-IO: Handle I/O errors
- REQ-LLR-ERR-MEM: Handle memory errors

**Verification Method:** Test
**Verification Criteria:**
- No crashes on invalid input
- Error messages are clear and actionable
- Errors reported via appropriate mechanism (exception, return code, log)

**Safety Impact:** High
**DO-278A Traceability:** Objective 1.1

---

### REQ-HLR-ERR-002: Error Reporting

**Category:** Functional
**Priority:** High
**Source:** User Requirements

**Description:**
The software shall provide clear, actionable error messages indicating:
- Type of error
- Location of error (file, offset, data item)
- Suggested corrective action (when applicable)

**Rationale:**
Users need to understand what went wrong to take corrective action.

**Derived Requirements:**
- REQ-LLR-ERR-MSG: Format error messages

**Verification Method:** Test
**Verification Criteria:**
- Error messages contain required information
- Messages are understandable

**Safety Impact:** Low
**DO-278A Traceability:** Objective 1.1

---

## 8. Performance Requirements

### REQ-HLR-PERF-001: Real-Time Processing

**Category:** Performance
**Priority:** Medium
**Source:** Operational Requirements

**Description:**
The software should process ASTERIX data at a rate sufficient for real-time surveillance data streams (typically 1000+ messages/second).

**Note:** This is a performance goal, not a hard requirement for AL-3.

**Rationale:**
Real-time ATC systems require timely data processing.

**Verification Method:** Analysis, Demonstration
**Verification Criteria:**
- Benchmark tests demonstrate adequate throughput
- No significant latency in typical use cases

**Safety Impact:** Low
**DO-278A Traceability:** Not directly traced (performance goal)

---

### REQ-HLR-PERF-002: Memory Efficiency

**Category:** Performance
**Priority:** Medium
**Source:** Operational Requirements

**Description:**
The software shall not have memory leaks and shall use memory efficiently for processing large data files.

**Rationale:**
Long-running processes must not accumulate memory over time.

**Derived Requirements:**
- REQ-LLR-MEM-LEAK: No memory leaks

**Verification Method:** Test (Valgrind)
**Verification Criteria:**
- Valgrind reports zero leaks
- Memory usage stable over extended operation

**Safety Impact:** Medium
**DO-278A Traceability:** Objective 4.2 (Testing)

---
## 9. Requirements Summary

### System and API Requirements

| Requirement ID | Description | Priority | Safety Impact |
|----------------|-------------|----------|---------------|
| REQ-HLR-SYS-001 | ASTERIX Protocol Compliance | Critical | High |
| REQ-HLR-SYS-002 | Data Integrity Verification | Critical | High |
| REQ-HLR-SYS-003 | Multiple ASTERIX Categories | High | Medium |
| REQ-HLR-IO-001 | Multiple Input Formats | High | Medium |
| REQ-HLR-IO-002 | Multiple Output Formats | High | Low |
| REQ-HLR-IO-003 | Data Filtering | Medium | Low |
| REQ-HLR-API-001 | Python API | High | Medium |
| REQ-HLR-API-002 | C/C++ Library API | High | Medium |
| REQ-HLR-API-003 | Command-Line Interface | High | Low |
| REQ-HLR-API-004 | Rust API | High | Medium |
| REQ-HLR-CFG-001 | XML Configuration Files | Critical | High |
| REQ-HLR-CFG-002 | Default Configuration | High | Medium |
| REQ-HLR-CFG-003 | Custom Configuration | Medium | Low |
| REQ-HLR-ERR-001 | Graceful Error Handling | Critical | High |
| REQ-HLR-ERR-002 | Error Reporting | High | Low |
| REQ-HLR-PERF-001 | Real-Time Processing | Medium | Low |
| REQ-HLR-PERF-002 | Memory Efficiency | Medium | Medium |

### Category-Specific Requirements

| Requirement ID | Category | Description | Priority | Safety Impact |
|----------------|----------|-------------|----------|---------------|
| REQ-HLR-CAT-001 | CAT001 | Radar Detection Messages | High | Medium |
| REQ-HLR-CAT-002 | CAT002 | Monoradar Target Reports | High | High |
| REQ-HLR-CAT-004 | CAT004 | Safety Net Messages | Critical | High |
| REQ-HLR-CAT-008 | CAT008 | Monoradar Derived Weather Information | Medium | Medium |
| REQ-HLR-CAT-010 | CAT010 | Monoradar Service Messages | Medium | Medium |
| REQ-HLR-CAT-011 | CAT011 | Track Status Messages | High | Medium |
| REQ-HLR-CAT-015 | CAT015 | Broadcast Messages | Medium | Low |
| REQ-HLR-CAT-019 | CAT019 | Multilateration System Status Messages | High | Medium |
| REQ-HLR-CAT-020 | CAT020 | Multilateration Target Reports | Critical | High |
| REQ-HLR-CAT-021 | CAT021 | ADS-B Target Reports | Critical | High |
| REQ-HLR-CAT-023 | CAT023 | CNS/ATM Ground Station Status Reports | Medium | Medium |
| REQ-HLR-CAT-025 | CAT025 | CNS/ATM Ground System Status Reports | Medium | Medium |
| REQ-HLR-CAT-030 | CAT030 | ARTAS Service Messages | High | Medium |
| REQ-HLR-CAT-031 | CAT031 | ARTAS Track Status Messages | High | Medium |
| REQ-HLR-CAT-032 | CAT032 | Miniature Range Test Messages | Low | Low |
| REQ-HLR-CAT-034 | CAT034 | Monoradar Service Messages | High | Medium |
| REQ-HLR-CAT-048 | CAT048 | Transmission of Monoradar Target Reports | Critical | High |
| REQ-HLR-CAT-062 | CAT062 | System Track Data | Critical | High |
| REQ-HLR-CAT-063 | CAT063 | Sensor Status Messages | Medium | Medium |
| REQ-HLR-CAT-065 | CAT065 | SDPS Service Status Messages | High | Medium |
| REQ-HLR-CAT-205 | CAT205 | Multilateration System Status - Extended | Medium | Medium |
| REQ-HLR-CAT-240 | CAT240 | Video Transmission | Medium | Low |
| REQ-HLR-CAT-247 | CAT247 | Reserved for ADS-B Messages | Low | Low |
| REQ-HLR-CAT-252 | CAT252 | ADS-B Transponder Tracks | High | High |

**Total High-Level Requirements:** 41

**Breakdown:**
- System/API Requirements: 17
- Category Requirements: 24

**Priority Breakdown:**
- Critical: 10 (including system reqs)
- High: 17
- Medium: 12
- Low: 2

**Safety Impact Breakdown:**
- High: 14
- Medium: 19
- Low: 9

---

## 10. Requirements Allocation

### To Software Design
All requirements shall be allocated to software design elements in the Software Design Description (SDD).

### To Verification
All requirements shall have associated test cases in the Software Verification Cases and Procedures (SVCP).

### Traceability
Complete bidirectional traceability shall be maintained in the Requirements Traceability Matrix (RTM).

---

## Document Approval

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Requirements Engineer | TBD | 2025-10-17 | |
| Project Lead | TBD | | |
| QA Manager | TBD | | |

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-10-17 | Initial | Initial HLR creation with CAT048 focus |
| 2.0 | 2025-11-02 | Auto-updated | Added all 24 ASTERIX category HLRs |

---

## Next Steps

1. Complete Low-Level Requirements (LLR) for REQ-HLR-CAT-048
2. Add HLR for remaining ASTERIX categories
3. Create Requirements Traceability Matrix
4. Link requirements to design (SDD)
5. Create test cases for each requirement
