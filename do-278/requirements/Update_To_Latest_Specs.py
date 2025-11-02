#!/usr/bin/env python3
"""
Update ASTERIX category XMLs to latest versions from asterix-specs-converter
"""

import shutil
from pathlib import Path
import re

# Map of categories to update with their latest versions
UPDATES = {
    "001": "1.4",   # 1.2 -> 1.4
    "002": "1.1",   # 1.0 -> 1.1
    "008": "1.3",   # 1.0 -> 1.3
    "011": "1.3",   # 1.2 -> 1.3
    "034": "1.29",  # 1.27 -> 1.29
    "048": "1.30",  # 1.21 -> 1.30 (CRITICAL - most important category)
    "062": "1.19",  # 1.18 -> 1.19
    "063": "1.6",   # 1.3 -> 1.6
    "065": "1.5",   # 1.3 -> 1.5
    "247": "1.3",   # 1.2 -> 1.3
}

def update_categories():
    """Update categories to latest versions"""
    source_dir = Path("asterix-specs-converter/specs")
    dest_dir = Path("asterix/config")

    if not source_dir.exists():
        print(f"❌ Source directory not found: {source_dir}")
        return False

    if not dest_dir.exists():
        print(f"❌ Destination directory not found: {dest_dir}")
        return False

    print("=" * 80)
    print("ASTERIX CATEGORY SPECIFICATION UPDATE")
    print("=" * 80)
    print()

    updated_count = 0
    failed_count = 0

    for cat_num, target_version in UPDATES.items():
        # Format version for filename: "1.30" -> "1_30"
        version_filename = target_version.replace('.', '_')

        # Find source file
        source_pattern = f"asterix_cat{cat_num}_{version_filename}.xml"
        source_file = source_dir / source_pattern

        if not source_file.exists():
            print(f"❌ CAT{cat_num}: Source file not found: {source_file}")
            failed_count += 1
            continue

        # Find and remove old version(s)
        old_files = list(dest_dir.glob(f"asterix_cat{cat_num}_*.xml"))
        for old_file in old_files:
            old_version_match = re.search(r'_(\d+_\d+)\.xml$', old_file.name)
            old_version = old_version_match.group(1).replace('_', '.') if old_version_match else "unknown"
            print(f"🗑️  CAT{cat_num}: Removing old version v{old_version}")
            old_file.unlink()

        # Copy new version
        dest_file = dest_dir / source_pattern
        shutil.copy2(source_file, dest_file)

        print(f"✅ CAT{cat_num}: Updated to v{target_version}")
        print(f"   {source_file.name} -> {dest_file}")
        updated_count += 1
        print()

    print("=" * 80)
    print(f"✅ Successfully updated: {updated_count} categories")
    if failed_count > 0:
        print(f"❌ Failed to update: {failed_count} categories")
    print("=" * 80)
    print()

    return updated_count > 0

def main():
    if update_categories():
        print("Next steps:")
        print("1. Regenerate LLRs: python3 do-278/requirements/Regenerate_Updated_LLRs.py")
        print("2. Verify: python3 do-278/requirements/Verify_LLR_Accuracy.py")
        print("3. Commit changes to asterix/config/")
        return 0
    else:
        print("❌ Update failed")
        return 1

if __name__ == "__main__":
    exit(main())
