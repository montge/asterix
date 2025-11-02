#!/usr/bin/env python3
"""
LLR Verification Tool - Verifies Low-Level Requirements against ASTERIX XML definitions
Generates traceability report for DO-278A compliance
"""

import xml.etree.ElementTree as ET
from pathlib import Path
from collections import defaultdict
import re

def parse_asterix_xml(xml_path):
    """Parse ASTERIX XML and extract all data items with details"""
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

            item_name = name_elem.text.strip() if name_elem is not None and name_elem.text else f"Data Item {item_id}"
            item_def = def_elem.text.strip() if def_elem is not None and def_elem.text else ""

            # Determine format type with details
            format_info = {"type": "Unknown", "details": ""}
            if fmt_elem is not None:
                if fmt_elem.find('Fixed') is not None:
                    fixed = fmt_elem.find('Fixed')
                    length = fixed.get('length', 'unknown')
                    format_info = {"type": "Fixed", "details": f"{length} bytes"}
                elif fmt_elem.find('Variable') is not None:
                    format_info = {"type": "Variable", "details": "variable length with FX"}
                elif fmt_elem.find('Repetitive') is not None:
                    rep = fmt_elem.find('Repetitive')
                    format_info = {"type": "Repetitive", "details": "REP + elements"}
                elif fmt_elem.find('Compound') is not None:
                    format_info = {"type": "Compound", "details": "FSPEC + subfields"}
                elif fmt_elem.find('Explicit') is not None:
                    format_info = {"type": "Explicit", "details": "LEN + data"}

            data_items.append({
                'id': item_id,
                'name': item_name,
                'definition': item_def,
                'format_type': format_info['type'],
                'format_details': format_info['details'],
                'rule': item_rule
            })

        return {
            'category': cat_id,
            'name': cat_name,
            'version': cat_ver,
            'data_items': data_items
        }
    except Exception as e:
        return None

def parse_llr_document(llr_path):
    """Parse LLR markdown document to extract requirements"""
    if not llr_path.exists():
        return None

    with open(llr_path, 'r') as f:
        content = f.read()

    # Extract category number from filename
    match = re.search(r'CAT(\d+)', llr_path.name)
    if not match:
        return None

    cat_num = match.group(1)

    # Extract requirements using regex (match category number as-is with leading zeros)
    req_pattern = r'### REQ-LLR-' + cat_num + r'-(\d+|SP|RE|RFS):'
    requirements = re.findall(req_pattern, content)

    # Extract version from document
    version_match = re.search(r'v(\d+\.\d+)', content)
    version = version_match.group(1) if version_match else "unknown"

    return {
        'category': cat_num,
        'version': version,
        'requirements': list(set(requirements))  # unique requirements
    }

def compare_xml_to_llr(xml_data, llr_data):
    """Compare XML data items with LLR requirements"""
    if not xml_data or not llr_data:
        return None

    xml_items = {item['id'] for item in xml_data['data_items']}
    llr_items = set(llr_data['requirements'])

    # Find mismatches
    missing_in_llr = xml_items - llr_items
    extra_in_llr = llr_items - xml_items
    matching = xml_items & llr_items

    return {
        'category': xml_data['category'],
        'xml_version': xml_data['version'],
        'llr_version': llr_data['version'],
        'xml_count': len(xml_items),
        'llr_count': len(llr_items),
        'matching_count': len(matching),
        'missing_in_llr': sorted(missing_in_llr),
        'extra_in_llr': sorted(extra_in_llr),
        'coverage_percent': round(len(matching) / len(xml_items) * 100, 1) if xml_items else 0,
        'xml_items_detail': xml_data['data_items']
    }

def check_newer_versions(current_cat, current_ver, converter_dir):
    """Check if newer versions exist in asterix-specs-converter/specs/"""
    cat_pattern = f"asterix_cat{current_cat}_*.xml"
    specs_path = Path(converter_dir) / "specs"

    if not specs_path.exists():
        return []

    available_versions = []
    for xml_file in specs_path.glob(cat_pattern):
        # Extract version from filename: asterix_cat048_1_27.xml -> 1.27
        match = re.search(r'_(\d+_\d+)\.xml$', xml_file.name)
        if match:
            version_str = match.group(1).replace('_', '.')
            available_versions.append({
                'version': version_str,
                'file': xml_file.name,
                'path': str(xml_file)
            })

    # Sort by version
    available_versions.sort(key=lambda x: [int(n) for n in x['version'].split('.')])

    # Find newer versions
    try:
        current_parts = [int(x) for x in current_ver.split('.')]
        newer = []
        for av in available_versions:
            av_parts = [int(x) for x in av['version'].split('.')]
            if av_parts > current_parts:
                newer.append(av)
        return newer
    except:
        return []

def generate_verification_report(config_dir, llr_dir, converter_dir, output_path):
    """Generate comprehensive verification report"""
    config_path = Path(config_dir)
    llr_path = Path(llr_dir)

    # Collect all data
    results = []
    categories_analyzed = []

    for xml_file in sorted(config_path.glob("asterix_cat*.xml")):
        # Skip BDS file
        if 'bds' in xml_file.name.lower():
            continue

        # Extract category number
        match = re.search(r'cat(\d+)', xml_file.name)
        if not match:
            continue

        cat_num = match.group(1)
        categories_analyzed.append(cat_num)

        # Parse XML
        xml_data = parse_asterix_xml(xml_file)
        if not xml_data:
            continue

        # Parse corresponding LLR
        llr_file = llr_path / f"Low_Level_Requirements_CAT{cat_num}.md"
        llr_data = parse_llr_document(llr_file)

        # Compare
        comparison = compare_xml_to_llr(xml_data, llr_data)
        if comparison:
            # Check for newer versions
            newer_versions = check_newer_versions(cat_num, xml_data['version'], converter_dir)
            comparison['newer_versions'] = newer_versions
            results.append(comparison)

    # Generate markdown report
    report = generate_markdown_report(results, categories_analyzed)

    # Write report
    with open(output_path, 'w') as f:
        f.write(report)

    return results

def generate_markdown_report(results, categories_analyzed):
    """Generate markdown verification report"""
    from datetime import date
    today = date.today().strftime("%Y-%m-%d")

    # Calculate statistics
    total_cats = len(results)
    perfect_match = sum(1 for r in results if r['coverage_percent'] == 100)
    total_xml_items = sum(r['xml_count'] for r in results)
    total_llr_items = sum(r['llr_count'] for r in results)
    total_matching = sum(r['matching_count'] for r in results)
    overall_coverage = round(total_matching / total_xml_items * 100, 1) if total_xml_items > 0 else 0

    updates_available = sum(1 for r in results if r['newer_versions'])

    report = f"""# LLR Verification and Traceability Report
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-VERIFY-001
**Verification Date:** {today}
**Purpose:** Verify Low-Level Requirements accuracy against ASTERIX XML definitions

---

## Executive Summary

This report verifies that generated Low-Level Requirements (LLR) documents accurately reflect the ASTERIX category XML definitions. It provides traceability from XML specifications to requirements documentation.

**Verification Status:**
- ✅ Categories Analyzed: **{total_cats}**
- ✅ Perfect Matches: **{perfect_match}/{total_cats}** ({round(perfect_match/total_cats*100, 1)}%)
- ✅ Overall Coverage: **{overall_coverage}%**
- ⚠️ Updates Available: **{updates_available}** categories have newer versions

**Statistics:**
| Metric | Count |
|--------|-------|
| Total XML Data Items | {total_xml_items} |
| Total LLR Requirements | {total_llr_items} |
| Matching Requirements | {total_matching} |
| **Overall Coverage** | **{overall_coverage}%** |

---

## Verification Results by Category

"""

    # Sort by coverage (worst first for attention)
    sorted_results = sorted(results, key=lambda x: x['coverage_percent'])

    for result in sorted_results:
        cat = result['category']
        status_icon = "✅" if result['coverage_percent'] == 100 else "⚠️"

        report += f"""### {status_icon} Category {cat} - {result['xml_items_detail'][0]['name'] if result['xml_items_detail'] else 'Unknown'} v{result['xml_version']}

**Coverage:** {result['coverage_percent']}% ({result['matching_count']}/{result['xml_count']} data items)
**LLR Status:** {"✅ Complete" if result['coverage_percent'] == 100 else "⚠️ Incomplete"}

| Metric | Value |
|--------|-------|
| XML Data Items | {result['xml_count']} |
| LLR Requirements | {result['llr_count']} |
| Matching | {result['matching_count']} |
| Missing in LLR | {len(result['missing_in_llr'])} |
| Extra in LLR | {len(result['extra_in_llr'])} |

"""

        if result['missing_in_llr']:
            report += f"**⚠️ Missing in LLR:** {', '.join(result['missing_in_llr'])}\n\n"

        if result['extra_in_llr']:
            report += f"**ℹ️ Extra in LLR (not in XML):** {', '.join(result['extra_in_llr'])}\n\n"

        if result['newer_versions']:
            report += f"**🔄 Newer Versions Available:**\n"
            for nv in result['newer_versions']:
                report += f"- v{nv['version']} ({nv['file']})\n"
            report += "\n"

        report += "---\n\n"

    # Add detailed traceability table
    report += """## Detailed Traceability Matrix

This table provides complete traceability from ASTERIX XML data items to LLR requirements.

| Category | Data Item | Name | Format | Rule | LLR Req | Status |
|----------|-----------|------|--------|------|---------|--------|
"""

    for result in sorted(results, key=lambda x: int(x['category'])):
        cat = result['category']
        for item in result['xml_items_detail']:
            item_id = item['id']
            llr_req = f"REQ-LLR-{cat}-{item_id}"
            status = "✅" if item_id in str(result['matching_count']) or item_id not in result['missing_in_llr'] else "❌"

            # Truncate long names
            name = item['name'][:40] + "..." if len(item['name']) > 40 else item['name']

            report += f"| CAT{cat} | I{cat}/{item_id} | {name} | {item['format_type']} | {item['rule']} | {llr_req} | {status} |\n"

    report += """

---

## Recommendations

"""

    # Generate recommendations
    incomplete_cats = [r for r in results if r['coverage_percent'] < 100]
    if incomplete_cats:
        report += f"""### 1. Complete LLR Coverage

The following categories have incomplete LLR coverage and should be regenerated:

"""
        for r in incomplete_cats:
            report += f"- **CAT{r['category']}**: {r['coverage_percent']}% coverage ({len(r['missing_in_llr'])} missing items)\n"

    if updates_available > 0:
        report += f"""

### 2. Update to Newer Specifications

**{updates_available}** categories have newer versions available in `asterix-specs-converter/specs/`:

"""
        for r in results:
            if r['newer_versions']:
                report += f"- **CAT{r['category']}**: Currently v{r['xml_version']}, available up to v{r['newer_versions'][-1]['version']}\n"

    report += """

### 3. Regenerate LLRs After Updates

After updating XML specifications:
1. Run `python3 do-278/requirements/Low_Level_Requirements_Generator.py`
2. Review generated requirements
3. Re-run this verification tool
4. Update Requirements Traceability Matrix

---

## Verification Methodology

### Process
1. **Parse XML Definitions**: Extract all DataItem elements from ASTERIX category XMLs
2. **Parse LLR Documents**: Extract REQ-LLR-xxx-yyy requirements from markdown
3. **Compare**: Match XML data items to LLR requirements by ID
4. **Report**: Generate coverage statistics and identify gaps

### Acceptance Criteria
- ✅ 100% coverage: Every XML data item has corresponding LLR
- ✅ No extra requirements: No LLRs without corresponding XML items
- ✅ Version alignment: LLR documents reference correct XML version

---

## Document Approval

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Requirements Engineer | TBD | {today} | |
| QA Manager | TBD | | |

---

## References

1. Low-Level Requirements documents (do-278/requirements/Low_Level_Requirements_CAT*.md)
2. ASTERIX XML definitions (asterix/config/asterix_cat*.xml)
3. Requirements Traceability Matrix (Requirements_Traceability_Matrix.md)
4. DO-278A Section 6.3.1 (Requirements Verification)
"""

    return report

def main():
    """Main verification function"""
    config_dir = "asterix/config"
    llr_dir = "do-278/requirements"
    converter_dir = "asterix-specs-converter"
    output_path = "do-278/requirements/LLR_Verification_Report.md"

    print("=" * 80)
    print("LLR VERIFICATION TOOL - DO-278A Compliance")
    print("=" * 80)
    print()
    print("Verifying Low-Level Requirements against ASTERIX XML definitions...")
    print()

    results = generate_verification_report(config_dir, llr_dir, converter_dir, output_path)

    if results:
        total = len(results)
        perfect = sum(1 for r in results if r['coverage_percent'] == 100)
        updates = sum(1 for r in results if r['newer_versions'])

        print(f"✅ Verification Complete!")
        print()
        print(f"Categories Analyzed: {total}")
        print(f"Perfect Matches:     {perfect}/{total} ({round(perfect/total*100, 1)}%)")
        print(f"Updates Available:   {updates} categories")
        print()
        print(f"📄 Report generated: {output_path}")
        print()

        if perfect < total:
            print("⚠️  Some categories have incomplete coverage - see report for details")

        if updates > 0:
            print(f"🔄 {updates} categories have newer versions available")
    else:
        print("❌ Verification failed - no results generated")
        return 1

    return 0

if __name__ == "__main__":
    exit(main())
