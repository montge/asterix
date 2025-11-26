#!/usr/bin/env python3
"""
Summarize Bandit security scan report.
"""

import json
import sys
from pathlib import Path


def summarize_report(report_path: str) -> int:
    """Summarize a Bandit JSON report."""
    path = Path(report_path)
    if not path.exists():
        print(f"Report not found: {report_path}")
        return 1

    try:
        with open(path) as f:
            data = json.load(f)
    except json.JSONDecodeError as e:
        print(f"Failed to parse report: {e}")
        return 1

    # Get metrics
    metrics = data.get('metrics', {})
    results = data.get('results', [])

    # Count by severity
    totals = metrics.get('_totals', {})
    high = totals.get('SEVERITY.HIGH', 0)
    medium = totals.get('SEVERITY.MEDIUM', 0)
    low = totals.get('SEVERITY.LOW', 0)

    print(f"=== Bandit Security Scan Summary ===")
    print(f"Report: {report_path}")
    print(f"Total issues: {len(results)}")
    print(f"  HIGH:   {high}")
    print(f"  MEDIUM: {medium}")
    print(f"  LOW:    {low}")

    # Show first few issues
    if results:
        print(f"\nTop issues:")
        for issue in results[:5]:
            severity = issue.get('issue_severity', 'UNKNOWN')
            confidence = issue.get('issue_confidence', 'UNKNOWN')
            text = issue.get('issue_text', 'No description')
            filename = issue.get('filename', 'unknown')
            line = issue.get('line_number', 0)
            print(f"  [{severity}/{confidence}] {filename}:{line}")
            print(f"    {text[:80]}...")

        if len(results) > 5:
            print(f"\n  ... and {len(results) - 5} more issues")

    return 0 if high == 0 else 1  # Fail only on HIGH severity


def main():
    if len(sys.argv) < 2:
        print("Usage: summarize_bandit_report.py <report.json> [report2.json ...]")
        return 1

    exit_code = 0
    for report_path in sys.argv[1:]:
        result = summarize_report(report_path)
        if result != 0:
            exit_code = result
        print()

    return exit_code


if __name__ == "__main__":
    sys.exit(main())
