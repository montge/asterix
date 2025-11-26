#!/usr/bin/env python3
"""
Summarize pip-audit vulnerability scan report.
"""

import json
import sys
from pathlib import Path


def main():
    report_path = sys.argv[1] if len(sys.argv) > 1 else "pip-audit-report.json"
    path = Path(report_path)

    if not path.exists():
        print(f"Report not found: {report_path}")
        print("No vulnerabilities found (no report)")
        return 0

    try:
        with open(path) as f:
            data = json.load(f)
    except json.JSONDecodeError as e:
        print(f"Failed to parse report: {e}")
        return 1

    print("=== pip-audit Vulnerability Scan Summary ===")

    # Handle both list format and dict format
    deps = data if isinstance(data, list) else data.get('dependencies', [])
    vulns = [d for d in deps if d.get('vulns')]

    if vulns:
        print(f"Vulnerable packages: {len(vulns)}")
        for dep in vulns[:10]:
            name = dep.get('name', 'unknown')
            version = dep.get('version', 'unknown')
            vuln_count = len(dep.get('vulns', []))
            print(f"  - {name} {version}: {vuln_count} vulnerabilities")

        if len(vulns) > 10:
            print(f"  ... and {len(vulns) - 10} more")

        return 0  # Informational, don't fail
    else:
        print("No vulnerabilities found")
        return 0


if __name__ == "__main__":
    sys.exit(main())
