# Software Requirements Template

This template is used for documenting both High-Level Requirements (HLR) and Low-Level Requirements (LLR) for the ASTERIX Decoder.

## High-Level Requirements (HLR)

High-level requirements define WHAT the software shall do from a system perspective. They are typically derived from:
- ASTERIX protocol specifications
- User needs
- System requirements
- Safety requirements

### HLR Format

```
REQ-HLR-XXX: [Requirement Title]

Category: [Functional | Performance | Interface | Safety | Data]
Priority: [Critical | High | Medium | Low]
Source: [ASTERIX Spec CAT-XXX | User Need | System Req]

Description:
The software shall [specific, testable requirement statement].

Rationale:
[Why this requirement exists]

Derived Requirements:
- REQ-LLR-XXX
- REQ-LLR-YYY

Verification Method: [Test | Analysis | Inspection | Demonstration]
Verification Criteria:
[How to verify this requirement is met]

Safety Impact: [None | Low | Medium | High]
DO-278A Traceability: [Applicable objective]
```

### HLR Example

```
REQ-HLR-001: Parse ASTERIX Category 048 Data Blocks

Category: Functional
Priority: Critical
Source: ASTERIX CAT048 Specification v1.30

Description:
The software shall parse ASTERIX Category 048 (Monoradar Target Reports) data blocks
according to EUROCONTROL specification CAT048 v1.30, extracting all defined data items.

Rationale:
Category 048 is a fundamental radar surveillance category used in ATC systems.
Correct parsing is critical for accurate radar track display.

Derived Requirements:
- REQ-LLR-048-001: Parse CAT048 FSPEC
- REQ-LLR-048-002: Parse CAT048 Data Item I048/010 (Data Source Identifier)
- REQ-LLR-048-020: Parse CAT048 Data Item I048/020 (Target Report Descriptor)
- [... more LLRs for each data item ...]

Verification Method: Test
Verification Criteria:
- All CAT048 data items correctly extracted
- CRC validation passes
- Output matches reference implementation
- Test with sample data: install/sample_data/cat_034_048.pcap

Safety Impact: High
DO-278A Traceability: Objective 1.1 (High-level requirements)
```

---

## Low-Level Requirements (LLR)

Low-level requirements define HOW the software implements the high-level requirements. They are more detailed and implementation-specific.

### LLR Format

```
REQ-LLR-XXX: [Requirement Title]

Parent: REQ-HLR-XXX
Category: [Functional | Algorithm | Interface | Error Handling]
Priority: [Critical | High | Medium | Low]

Description:
The [component/function] shall [specific, testable requirement statement].

Implementation Notes:
[Technical details, algorithms, data structures]

Verification Method: [Unit Test | Integration Test | Code Review]
Test Case(s): TC-XXX, TC-YYY

Code Reference: [file.cpp:line or function name]
Design Reference: [SDD section]

Safety Impact: [None | Low | Medium | High]
```

### LLR Example

```
REQ-LLR-048-010: Parse ASTERIX CAT048 Data Item I048/010

Parent: REQ-HLR-001
Category: Functional
Priority: Critical

Description:
The parser shall extract Data Item I048/010 (Data Source Identifier) as a 2-byte
fixed-length field containing SAC (System Area Code) and SIC (System Identification Code).

Implementation Notes:
- Fixed format: 2 bytes
- Byte 1: SAC (8 bits, unsigned)
- Byte 2: SIC (8 bits, unsigned)
- No scaling required (raw values)
- Implemented in: DataItemFormatFixed::parse()

Verification Method: Unit Test
Test Case(s):
- TC-048-010-001: Valid SAC/SIC values
- TC-048-010-002: Boundary values (0x00, 0xFF)
- TC-048-010-003: Multiple occurrences in data stream

Code Reference:
- src/asterix/DataItemFormatFixed.cpp:145
- asterix/config/asterix_cat048_*.xml

Design Reference: SDD Section 3.4.2 (Fixed Format Parsing)

Safety Impact: High (incorrect SAC/SIC could misidentify data source)
```

---

## Requirements Attributes

### Category
- **Functional**: What the system does
- **Performance**: Speed, throughput, latency
- **Interface**: External interactions
- **Safety**: Safety-related constraints
- **Data**: Data format, storage, integrity
- **Algorithm**: Computation methods
- **Error Handling**: Fault tolerance, recovery

### Priority
- **Critical**: Core functionality, safety-critical
- **High**: Important features, frequently used
- **Medium**: Standard features
- **Low**: Nice-to-have, rarely used

### Verification Method
- **Test**: Automated or manual testing
- **Analysis**: Mathematical or logical proof
- **Inspection**: Code review, document review
- **Demonstration**: Show functionality to stakeholders

### Safety Impact
- **High**: Direct impact on surveillance data accuracy
- **Medium**: Could lead to degraded performance
- **Low**: Minimal safety impact
- **None**: No safety relevance

---

## Requirements Traceability

Every requirement must link to:
- **Parent** (for LLR → HLR)
- **Design** (SDD section)
- **Code** (source file and function)
- **Test** (test case ID)

Example traceability chain:
```
REQ-HLR-001 (Parse CAT048)
  ↓
REQ-LLR-048-010 (Parse I048/010)
  ↓
SDD-3.4.2 (Fixed Format Parser Design)
  ↓
DataItemFormatFixed::parse() (src/asterix/DataItemFormatFixed.cpp:145)
  ↓
TC-048-010-001 (Test SAC/SIC parsing)
```

---

## ASTERIX-Specific Requirements

Requirements derived from ASTERIX specifications should reference:

- **Category**: CAT001, CAT002, ..., CAT252
- **Edition**: Version of the ASTERIX spec (e.g., v1.30, v1.18)
- **Data Item**: I048/010, I062/040, etc.
- **UAP**: User Application Profile
- **Format**: Fixed, Variable, Compound, Repetitive, Explicit, BDS

---

## Next Steps

1. **Extract HLRs** from ASTERIX specifications:
   - One HLR per ASTERIX category (REQ-HLR-048, REQ-HLR-062, etc.)
   - One HLR per major feature (parsing, I/O, formatting)

2. **Derive LLRs** from HLRs:
   - One LLR per data item
   - One LLR per algorithm
   - One LLR per interface

3. **Create Test Cases** for each LLR:
   - Normal operation
   - Boundary conditions
   - Error conditions

4. **Establish Traceability**:
   - Link all requirements to tests
   - Link all code to requirements
   - Verify 100% coverage

5. **Measure Coverage**:
   - Run tests with coverage enabled
   - Identify gaps
   - Add tests to reach 90-95% target
