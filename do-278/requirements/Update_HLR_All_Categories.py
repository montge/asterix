#!/usr/bin/env python3
"""
Update High-Level Requirements to include all 24 ASTERIX categories
"""

from pathlib import Path
from datetime import date

# Category information (from generator)
CATEGORIES = {
    "001": {"name": "Radar Detection Messages", "priority": "High", "safety": "Medium"},
    "002": {"name": "Monoradar Target Reports", "priority": "High", "safety": "High"},
    "004": {"name": "Safety Net Messages", "priority": "Critical", "safety": "High"},
    "008": {"name": "Monoradar Derived Weather Information", "priority": "Medium", "safety": "Medium"},
    "010": {"name": "Monoradar Service Messages", "priority": "Medium", "safety": "Medium"},
    "011": {"name": "Track Status Messages", "priority": "High", "safety": "Medium"},
    "015": {"name": "Broadcast Messages", "priority": "Medium", "safety": "Low"},
    "019": {"name": "Multilateration System Status Messages", "priority": "High", "safety": "Medium"},
    "020": {"name": "Multilateration Target Reports", "priority": "Critical", "safety": "High"},
    "021": {"name": "ADS-B Target Reports", "priority": "Critical", "safety": "High"},
    "023": {"name": "CNS/ATM Ground Station Status Reports", "priority": "Medium", "safety": "Medium"},
    "025": {"name": "CNS/ATM Ground System Status Reports", "priority": "Medium", "safety": "Medium"},
    "030": {"name": "ARTAS Service Messages", "priority": "High", "safety": "Medium"},
    "031": {"name": "ARTAS Track Status Messages", "priority": "High", "safety": "Medium"},
    "032": {"name": "Miniature Range Test Messages", "priority": "Low", "safety": "Low"},
    "034": {"name": "Monoradar Service Messages", "priority": "High", "safety": "Medium"},
    "048": {"name": "Transmission of Monoradar Target Reports", "priority": "Critical", "safety": "High"},
    "062": {"name": "System Track Data", "priority": "Critical", "safety": "High"},
    "063": {"name": "Sensor Status Messages", "priority": "Medium", "safety": "Medium"},
    "065": {"name": "SDPS Service Status Messages", "priority": "High", "safety": "Medium"},
    "205": {"name": "Multilateration System Status - Extended", "priority": "Medium", "safety": "Medium"},
    "240": {"name": "Video Transmission", "priority": "Medium", "safety": "Low"},
    "247": {"name": "Reserved for ADS-B Messages", "priority": "Low", "safety": "Low"},
    "252": {"name": "ADS-B Transponder Tracks", "priority": "High", "safety": "High"},
}

def generate_category_hlr_section():
    """Generate HLR section for all categories"""
    section = "\n## 3. Category-Specific Requirements\n\n"

    for cat_num in sorted(CATEGORIES.keys()):
        cat_info = CATEGORIES[cat_num]
        cat_name = cat_info['name']
        priority = cat_info['priority']
        safety = cat_info['safety']

        section += f"""### REQ-HLR-CAT-{cat_num}: Parse ASTERIX Category {cat_num}

**Category:** Functional
**Priority:** {priority}
**Source:** EUROCONTROL ASTERIX CAT{cat_num} Specification

**Description:**
The software shall parse ASTERIX Category {cat_num} ({cat_name}) data blocks according to EUROCONTROL specification, extracting all defined data items.

**Rationale:**
Category {cat_num} provides {cat_name.lower()} essential for ATM surveillance operations.

**Derived Requirements:**
- See Low_Level_Requirements_CAT{cat_num}.md for detailed data item requirements

**Verification Method:** Test
**Verification Criteria:**
- All CAT{cat_num} data items correctly extracted
- Values match EUROCONTROL reference data
- Integration test with sample data

**Safety Impact:** {safety}
**DO-278A Traceability:** Objective 1.1

---

"""

    return section

def generate_summary_table():
    """Generate updated summary table with all categories"""
    table = """## 9. Requirements Summary

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
"""

    for cat_num in sorted(CATEGORIES.keys()):
        cat_info = CATEGORIES[cat_num]
        table += f"| REQ-HLR-CAT-{cat_num} | CAT{cat_num} | {cat_info['name']} | {cat_info['priority']} | {cat_info['safety']} |\n"

    total_sys = 17
    total_cat = len(CATEGORIES)
    total_all = total_sys + total_cat

    table += f"""
**Total High-Level Requirements:** {total_all}

**Breakdown:**
- System/API Requirements: {total_sys}
- Category Requirements: {total_cat}

**Priority Breakdown:**
- Critical: {sum(1 for c in CATEGORIES.values() if c['priority'] == 'Critical') + 5} (including system reqs)
- High: {sum(1 for c in CATEGORIES.values() if c['priority'] == 'High') + 8}
- Medium: {sum(1 for c in CATEGORIES.values() if c['priority'] == 'Medium') + 4}
- Low: {sum(1 for c in CATEGORIES.values() if c['priority'] == 'Low')}

**Safety Impact Breakdown:**
- High: {sum(1 for c in CATEGORIES.values() if c['safety'] == 'High') + 7}
- Medium: {sum(1 for c in CATEGORIES.values() if c['safety'] == 'Medium') + 6}
- Low: {sum(1 for c in CATEGORIES.values() if c['safety'] == 'Low') + 5}

---
"""

    return table

def append_to_hlr():
    """Append new sections to High-Level Requirements"""
    hlr_path = Path("do-278/requirements/High_Level_Requirements.md")

    if not hlr_path.exists():
        print(f"HLR file not found: {hlr_path}")
        return

    # Read existing content
    with open(hlr_path, 'r') as f:
        content = f.read()

    # Check if we need to update
    if "REQ-HLR-CAT-001" in content:
        print("HLR already contains all categories - skipping update")
        return

    # Find insertion point (before section 4)
    insert_marker = "\n## 4. Input/Output Requirements"

    if insert_marker not in content:
        print("Could not find insertion point in HLR")
        return

    # Generate new content
    category_section = generate_category_hlr_section()

    # Insert category section
    content = content.replace(insert_marker, category_section + insert_marker)

    # Replace summary section (find and replace section 9)
    summary_marker_start = "\n## 9. Requirements Summary"
    summary_marker_end = "\n## 10. Requirements Allocation"

    if summary_marker_start in content and summary_marker_end in content:
        start_idx = content.find(summary_marker_start)
        end_idx = content.find(summary_marker_end)
        new_summary = generate_summary_table()
        content = content[:start_idx] + new_summary + content[end_idx:]

    # Update document history
    today = date.today().strftime("%Y-%m-%d")
    history_marker = "| 1.0 | 2025-10-17 | Initial | Initial HLR creation with CAT048 focus |"
    new_history = f"""| 1.0 | 2025-10-17 | Initial | Initial HLR creation with CAT048 focus |
| 2.0 | {today} | Auto-updated | Added all 24 ASTERIX category HLRs |"""
    content = content.replace(history_marker, new_history)

    # Update revision
    content = content.replace("**Revision:** 1.0", "**Revision:** 2.0")

    # Write updated content
    with open(hlr_path, 'w') as f:
        f.write(content)

    print(f"✅ Updated {hlr_path}")
    print(f"   Added {len(CATEGORIES)} category-specific HLRs")

def main():
    append_to_hlr()

if __name__ == "__main__":
    main()
