#!/usr/bin/env python3
"""
Regenerate LLRs for updated ASTERIX categories
"""

from Low_Level_Requirements_Generator import parse_asterix_xml, generate_llr_document
from pathlib import Path

# Categories that were updated
UPDATED_CATEGORIES = ["001", "002", "008", "011", "034", "048", "062", "063", "065", "247"]

def main():
    config_dir = Path("asterix/config")
    output_dir = Path("do-278/requirements")

    print("=" * 80)
    print("REGENERATING LLRs FOR UPDATED CATEGORIES")
    print("=" * 80)
    print()

    regenerated_count = 0

    for cat_num in UPDATED_CATEGORIES:
        # Find the XML file for this category
        xml_files = list(config_dir.glob(f"asterix_cat{cat_num}_*.xml"))

        if not xml_files:
            print(f"❌ CAT{cat_num}: No XML file found")
            continue

        xml_file = xml_files[0]  # Should only be one after update

        # Parse XML
        cat_data = parse_asterix_xml(xml_file)
        if not cat_data:
            print(f"❌ CAT{cat_num}: Failed to parse XML")
            continue

        # Generate LLR (overwrite existing)
        output_file = output_dir / f"Low_Level_Requirements_CAT{cat_num}.md"

        # Backup existing if it exists
        if output_file.exists():
            backup_file = output_dir / f"Low_Level_Requirements_CAT{cat_num}_old.md.bak"
            output_file.rename(backup_file)
            print(f"📦 CAT{cat_num}: Backed up old LLR to {backup_file.name}")

        generate_llr_document(cat_data, output_file)
        regenerated_count += 1
        print(f"✅ CAT{cat_num}: Regenerated LLR for v{cat_data['version']} ({len(cat_data['data_items'])} data items)")
        print()

    print("=" * 80)
    print(f"✅ Regenerated {regenerated_count} LLR documents")
    print("=" * 80)
    print()
    print("Next: Run verification to confirm 100% coverage")
    print("python3 do-278/requirements/Verify_LLR_Accuracy.py")

if __name__ == "__main__":
    main()
