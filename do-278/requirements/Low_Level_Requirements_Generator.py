#!/usr/bin/env python3
"""
DO-278A Low-Level Requirements Generator for ASTERIX Categories
Generates LLR documents from ASTERIX XML category definitions
"""

import xml.etree.ElementTree as ET
import os
from pathlib import Path
from datetime import date

# ASTERIX Category descriptions
CATEGORY_DESCRIPTIONS = {
    "001": "Radar Detection Messages",
    "002": "Monoradar Target Reports",
    "004": "Safety Net Messages",
    "008": "Monoradar Derived Weather Information",
    "010": "Monoradar Service Messages",
    "011": "Monoradar Service Messages - Track Status Messages",
    "015": "Broadcast Messages",
    "019": "Multilateration System Status Messages",
    "020": "Multilateration Target Reports",
    "021": "ADS-B Target Reports",
    "023": "CNS/ATM Ground Station Status Reports",
    "025": "CNS/ATM Ground System Status Reports",
    "030": "ARTAS Service Messages",
    "031": "ARTAS Service Messages - Track Status Messages",
    "032": "Miniature Range Test Messages",
    "034": "Monoradar Service Messages",
    "048": "Transmission of Monoradar Target Reports",
    "062": "System Track Data",
    "063": "Sensor Status Messages",
    "065": "SDPS Service Status Messages",
    "205": "Multilateration System Status Messages - Extended",
    "240": "Video Transmission",
    "247": "Reserved for ADS-B Messages",
    "252": "ADS-B Transponder Tracks",
}

# Safety impact levels based on category criticality
CATEGORY_SAFETY_LEVELS = {
    "048": "High",  # Monoradar targets - critical
    "062": "High",  # System tracks - critical
    "021": "High",  # ADS-B targets - critical
    "020": "High",  # Multilateration - critical
    "065": "Medium",  # Service status
    "034": "Medium",  # Service messages
    "063": "Medium",  # Sensor status
    "004": "High",  # Safety nets
    # Default to Medium for others
}

def get_category_number(filename):
    """Extract category number from filename"""
    # asterix_cat048_1_21.xml -> 048
    parts = filename.split('_')
    if len(parts) >= 2 and parts[1].startswith('cat'):
        return parts[1][3:]
    return None

def parse_asterix_xml(xml_path):
    """Parse ASTERIX XML and extract category info"""
    try:
        tree = ET.parse(xml_path)
        root = tree.getroot()

        cat_id = root.get('id')
        cat_name = root.get('name', 'Unknown')
        cat_ver = root.get('ver', '1.0')

        data_items = []
        for item in root.findall('DataItem'):
            item_id = item.get('id')
            item_rule = item.get('rule', 'optional')

            name_elem = item.find('DataItemName')
            def_elem = item.find('DataItemDefinition')
            fmt_elem = item.find('DataItemFormat')
            note_elem = item.find('DataItemNote')

            item_name = name_elem.text if name_elem is not None else f"Data Item {item_id}"
            item_def = def_elem.text if def_elem is not None else ""
            item_note = note_elem.text if note_elem is not None else ""

            # Determine format type
            item_format = "Unknown"
            if fmt_elem is not None:
                if fmt_elem.find('Fixed') is not None:
                    fixed = fmt_elem.find('Fixed')
                    length = fixed.get('length', 'variable')
                    item_format = f"Fixed ({length} bytes)"
                elif fmt_elem.find('Variable') is not None:
                    item_format = "Variable"
                elif fmt_elem.find('Repetitive') is not None:
                    item_format = "Repetitive"
                elif fmt_elem.find('Compound') is not None:
                    item_format = "Compound"
                elif fmt_elem.find('Explicit') is not None:
                    item_format = "Explicit"

            data_items.append({
                'id': item_id,
                'name': item_name,
                'definition': item_def,
                'format': item_format,
                'rule': item_rule,
                'note': item_note
            })

        return {
            'category': cat_id,
            'name': cat_name,
            'version': cat_ver,
            'data_items': data_items
        }
    except Exception as e:
        print(f"Error parsing {xml_path}: {e}")
        return None

def get_safety_impact(cat_num, item_rule):
    """Determine safety impact based on category and item rule"""
    base_impact = CATEGORY_SAFETY_LEVELS.get(cat_num, "Medium")

    # Mandatory items are more critical
    if item_rule == "mandatory":
        if base_impact == "Medium":
            return "High"
        return base_impact
    else:
        if base_impact == "High":
            return "Medium"
        return "Low"

def get_priority(item_rule, safety_impact):
    """Determine priority based on rule and safety impact"""
    if item_rule == "mandatory" or safety_impact == "High":
        return "Critical"
    elif safety_impact == "Medium":
        return "High"
    else:
        return "Medium"

def generate_llr_document(cat_data, output_path):
    """Generate Low-Level Requirements document"""
    cat_num = cat_data['category']
    cat_name = cat_data['name']
    cat_ver = cat_data['version']
    items = cat_data['data_items']

    today = date.today().strftime("%Y-%m-%d")

    # Build document content
    doc = f"""# Low-Level Requirements - ASTERIX Category {cat_num}
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-CAT{cat_num}-001
**Revision:** 1.0
**Date:** {today}
**Parent HLR:** REQ-HLR-CAT-{cat_num}
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

This document defines Low-Level Requirements (LLR) for parsing ASTERIX Category {cat_num} ({cat_name}) v{cat_ver}.

**Parent Requirement:** REQ-HLR-CAT-{cat_num} - Parse ASTERIX Category {cat_num}

Each data item in CAT{cat_num} has corresponding LLRs defining parsing behavior.

---

## 2. CAT{cat_num} Data Item Requirements

"""

    # Generate requirements for each data item
    for idx, item in enumerate(items, 1):
        item_id = item['id']
        item_name = item['name']
        item_def = item['definition']
        item_format = item['format']
        item_rule = item['rule']
        item_note = item['note']

        safety_impact = get_safety_impact(cat_num, item_rule)
        priority = get_priority(item_rule, safety_impact)

        doc += f"""### REQ-LLR-{cat_num}-{item_id}: Parse {item_name} (I{cat_num}/{item_id})

**Parent:** REQ-HLR-CAT-{cat_num}
**Category:** Functional
**Priority:** {priority}

**Description:**
The parser shall extract Data Item I{cat_num}/{item_id} ({item_name}) as specified in the EUROCONTROL ASTERIX CAT{cat_num} v{cat_ver} specification.

**Data Item Definition:**
{item_def}

**Implementation Notes:**
- Format: {item_format}
- Rule: {item_rule}
"""

        if item_note:
            doc += f"- Note: {item_note}\n"

        doc += f"""
**Verification Method:** Unit Test
**Test Cases:**
- TC-{cat_num}-{item_id}-001: Parse valid {item_name}
- TC-{cat_num}-{item_id}-002: Verify format compliance
- TC-{cat_num}-{item_id}-003: Test boundary values

**Code Reference:**
- src/asterix/DataItemFormat*.cpp (based on format type)
- asterix/config/asterix_cat{cat_num.zfill(3)}_*.xml

**Design Reference:** SDD Section 3.4 (Data Item Parsing)

**Safety Impact:** {safety_impact}

---

"""

    # Add error handling section
    doc += f"""## 3. Error Handling Requirements

### REQ-LLR-{cat_num}-ERR-001: Invalid Data Item Length

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** High

**Description:**
The parser shall detect and report errors when a CAT{cat_num} data item has invalid length or format.

**Verification Method:** Unit Test
**Test Cases:**
- TC-{cat_num}-ERR-001: Truncated data items
- TC-{cat_num}-ERR-002: Invalid repetition counts
- TC-{cat_num}-ERR-003: Malformed compound items

**Safety Impact:** High

---

### REQ-LLR-{cat_num}-ERR-002: Unknown Data Item

**Parent:** REQ-HLR-ERR-001
**Category:** Error Handling
**Priority:** Medium

**Description:**
The parser shall gracefully handle CAT{cat_num} data items not defined in the configuration.

**Verification Method:** Unit Test
**Test Cases:**
- TC-{cat_num}-ERR-004: Unknown data item in FSPEC

**Safety Impact:** Medium

---

## 4. Requirements Summary Table

| Requirement ID | Data Item | Description | Priority | Safety |
|----------------|-----------|-------------|----------|--------|
"""

    # Add summary table
    for item in items:
        item_id = item['id']
        item_name = item['name'][:40]  # Truncate long names
        safety = get_safety_impact(cat_num, item['rule'])
        priority = get_priority(item['rule'], safety)
        doc += f"| REQ-LLR-{cat_num}-{item_id} | I{cat_num}/{item_id} | {item_name} | {priority} | {safety} |\n"

    doc += f"""| REQ-LLR-{cat_num}-ERR-001 | - | Invalid Data Item Length | High | High |
| REQ-LLR-{cat_num}-ERR-002 | - | Unknown Data Item | Medium | Medium |

**Total Requirements:** {len(items) + 2}

---

## 5. Traceability

Each LLR traces to:
- **Parent HLR:** REQ-HLR-CAT-{cat_num}
- **Design:** Software Design Description (SDD)
- **Code:** Source files and configuration XML
- **Tests:** Specific test cases (TC-{cat_num}-xxx-xxx)

See Requirements Traceability Matrix (RTM) for complete mapping.

---

## Document Approval

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Requirements Engineer | TBD | {today} | |
| Developer | TBD | | |
| QA Manager | TBD | | |

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | {today} | Auto-generated | Initial LLR for CAT{cat_num} data items |

---

## Next Actions

1. Review and validate generated requirements
2. Create detailed test cases for each LLR
3. Link LLR to existing code implementation
4. Update Requirements Traceability Matrix
5. Implement missing unit tests
"""

    # Write to file
    with open(output_path, 'w') as f:
        f.write(doc)

    print(f"Generated: {output_path}")

def main():
    """Main function to generate all LLR documents"""
    config_dir = Path("asterix/config")
    output_dir = Path("do-278/requirements")

    if not config_dir.exists():
        print(f"Config directory not found: {config_dir}")
        return

    output_dir.mkdir(parents=True, exist_ok=True)

    # Find all ASTERIX category XML files
    xml_files = sorted(config_dir.glob("asterix_cat*.xml"))

    print(f"Found {len(xml_files)} ASTERIX category files")
    print("Generating Low-Level Requirements documents...")
    print()

    generated_count = 0
    for xml_file in xml_files:
        cat_num = get_category_number(xml_file.name)
        if not cat_num:
            continue

        # Skip if already exists (don't overwrite CAT048)
        output_file = output_dir / f"Low_Level_Requirements_CAT{cat_num}.md"
        if output_file.exists():
            print(f"Skipping CAT{cat_num} (already exists)")
            continue

        # Parse XML and generate LLR
        cat_data = parse_asterix_xml(xml_file)
        if cat_data:
            generate_llr_document(cat_data, output_file)
            generated_count += 1

    print()
    print(f"✅ Generated {generated_count} Low-Level Requirements documents")
    print(f"📁 Output directory: {output_dir}")

if __name__ == "__main__":
    main()
