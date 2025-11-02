#!/usr/bin/env python3
"""
Map Low-Level Requirements to actual test cases
Generates test coverage report for DO-278A compliance
"""

import re
import subprocess
from pathlib import Path
from collections import defaultdict

def extract_llr_requirements(llr_dir):
    """Extract all LLR requirements from documents"""
    requirements = defaultdict(list)

    for llr_file in Path(llr_dir).glob("Low_Level_Requirements_CAT*.md"):
        match = re.search(r'CAT(\d+)', llr_file.name)
        if not match:
            continue

        cat_num = match.group(1)

        with open(llr_file, 'r') as f:
            content = f.read()

        # Extract requirements
        req_pattern = r'### (REQ-LLR-' + cat_num + r'-\d+|REQ-LLR-' + cat_num + r'-(?:SP|RE|RFS|ERR-\d+)):'
        reqs = re.findall(req_pattern, content)

        requirements[cat_num] = reqs

    return requirements

def find_test_files():
    """Find all test files"""
    test_files = {
        'cpp_unit': list(Path('tests/cpp').glob('test_*.cpp')) if Path('tests/cpp').exists() else [],
        'cpp_integration': [Path('install/test/test.sh')] if Path('install/test/test.sh').exists() else [],
        'python_unit': list(Path('asterix/test').glob('test_*.py')) if Path('asterix/test').exists() else [],
        'rust_unit': list(Path('asterix-rs/tests').glob('*.rs')) if Path('asterix-rs/tests').exists() else [],
    }
    return test_files

def search_test_for_category(test_file, category):
    """Search test file for references to a category"""
    try:
        with open(test_file, 'r') as f:
            content = f.read()

        # Search for category references
        patterns = [
            rf'cat\s*0*{int(category)}\b',  # cat048, cat48, cat 48
            rf'CAT\s*0*{int(category)}\b',  # CAT048
            rf'category.*0*{int(category)}\b',  # category 048
            rf'asterix_cat0*{category}',  # asterix_cat048
        ]

        matches = []
        for pattern in patterns:
            if re.search(pattern, content, re.IGNORECASE):
                matches.append(pattern)

        return len(matches) > 0
    except:
        return False

def analyze_test_coverage(requirements, test_files):
    """Analyze which requirements have test coverage"""
    coverage = {}

    for cat_num, reqs in requirements.items():
        cat_tests = {
            'cpp_unit': [],
            'cpp_integration': [],
            'python_unit': [],
            'rust_unit': [],
        }

        # Check each test suite
        for suite_name, files in test_files.items():
            for test_file in files:
                if search_test_for_category(test_file, cat_num):
                    cat_tests[suite_name].append(test_file.name)

        coverage[cat_num] = {
            'requirements': reqs,
            'req_count': len(reqs),
            'tests': cat_tests,
            'has_cpp_tests': len(cat_tests['cpp_unit']) > 0,
            'has_integration_tests': len(cat_tests['cpp_integration']) > 0,
            'has_python_tests': len(cat_tests['python_unit']) > 0,
            'has_rust_tests': len(cat_tests['rust_unit']) > 0,
            'test_count': sum(len(v) for v in cat_tests.values()),
        }

    return coverage

def count_test_cases_in_file(test_file):
    """Count actual test cases in a file"""
    try:
        with open(test_file, 'r') as f:
            content = f.read()

        # Different patterns for different file types
        if test_file.suffix == '.cpp':
            # Google Test: TEST(...), TEST_F(...), TEST_P(...)
            test_pattern = r'TEST[_FP]*\s*\('
        elif test_file.suffix == '.py':
            # Python: def test_...(), class Test...
            test_pattern = r'def test_|class Test'
        elif test_file.suffix == '.rs':
            # Rust: #[test]
            test_pattern = r'#\[test\]'
        elif test_file.suffix == '.sh':
            # Shell: "=== Test ..."
            test_pattern = r'===\s*Test'
        else:
            return 0

        return len(re.findall(test_pattern, content))
    except:
        return 0

def generate_test_mapping_report(coverage, output_path):
    """Generate test mapping report"""
    from datetime import date
    today = date.today().strftime("%Y-%m-%d")

    # Calculate statistics
    total_cats = len(coverage)
    cats_with_tests = sum(1 for c in coverage.values() if c['test_count'] > 0)
    total_requirements = sum(c['req_count'] for c in coverage.values())

    cpp_coverage = sum(1 for c in coverage.values() if c['has_cpp_tests'])
    integration_coverage = sum(1 for c in coverage.values() if c['has_integration_tests'])
    python_coverage = sum(1 for c in coverage.values() if c['has_python_tests'])
    rust_coverage = sum(1 for c in coverage.values() if c['has_rust_tests'])

    report = f"""# LLR to Test Mapping Report
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** LLR-TEST-MAP-001
**Date:** {today}
**Purpose:** Map Low-Level Requirements to actual test cases

---

## Executive Summary

This report maps Low-Level Requirements (LLR) to existing test suites, identifying test coverage for DO-278A verification.

**Coverage Summary:**
- Categories Analyzed: **{total_cats}**
- Categories with Tests: **{cats_with_tests}/{total_cats}** ({round(cats_with_tests/total_cats*100, 1)}%)
- Total Requirements: **{total_requirements}**

**Test Suite Coverage:**
| Test Suite | Categories Covered | Coverage % |
|------------|-------------------|------------|
| C++ Unit Tests | {cpp_coverage}/{total_cats} | {round(cpp_coverage/total_cats*100, 1)}% |
| Integration Tests | {integration_coverage}/{total_cats} | {round(integration_coverage/total_cats*100, 1)}% |
| Python Unit Tests | {python_coverage}/{total_cats} | {round(python_coverage/total_cats*100, 1)}% |
| Rust Unit Tests | {rust_coverage}/{total_cats} | {round(rust_coverage/total_cats*100, 1)}% |

---

## Detailed Mapping by Category

"""

    # Sort by test coverage (least coverage first for attention)
    sorted_cats = sorted(coverage.items(), key=lambda x: x[1]['test_count'])

    for cat_num, cat_data in sorted_cats:
        status_icon = "✅" if cat_data['test_count'] > 0 else "❌"
        test_count = cat_data['test_count']

        report += f"""### {status_icon} Category {cat_num} ({cat_data['req_count']} requirements)

**Test Coverage:** {test_count} test suite(s) found

| Test Type | Files | Status |
|-----------|-------|--------|
"""

        for suite_name, files in cat_data['tests'].items():
            if files:
                status = "✅ Covered"
                files_str = ", ".join(files)
            else:
                status = "❌ No tests"
                files_str = "-"

            suite_display = suite_name.replace('_', ' ').title()
            report += f"| {suite_display} | {files_str} | {status} |\n"

        # List requirements for this category
        report += f"\n**Requirements ({cat_data['req_count']}):**\n"
        for req in cat_data['requirements'][:5]:  # Show first 5
            report += f"- {req}\n"

        if len(cat_data['requirements']) > 5:
            report += f"- ... and {len(cat_data['requirements']) - 5} more\n"

        report += "\n---\n\n"

    # Add gaps section
    report += """## Test Coverage Gaps

### Categories Without Tests

"""

    no_tests = [cat for cat, data in coverage.items() if data['test_count'] == 0]
    if no_tests:
        report += "The following categories have **no test coverage**:\n\n"
        for cat in sorted(no_tests, key=lambda x: int(x)):
            req_count = coverage[cat]['req_count']
            report += f"- **CAT{cat}**: {req_count} requirements, 0 tests\n"
    else:
        report += "✅ All categories have some test coverage!\n"

    report += """

### Recommended Actions

1. **Priority 1 (Critical)**: Add tests for CAT048 and CAT062 if missing
   - These are safety-critical categories (monoradar and system tracks)

2. **Priority 2 (High)**: Add tests for CAT020, CAT021 (ADS-B, multilateration)

3. **Priority 3 (Medium)**: Add tests for remaining categories

---

## Test Implementation Guidelines

### For Each Category
1. **Unit Tests** (`tests/cpp/test_category_XXX.cpp`):
   - Test each data item format (Fixed, Variable, Compound, Repetitive, Explicit)
   - Verify parsing accuracy
   - Test boundary conditions

2. **Integration Tests** (`install/test/test.sh`):
   - End-to-end parsing with sample data
   - Verify output format
   - Test with sample_data/cat_XXX_YYY.pcap

3. **API Tests**:
   - Python: `asterix/test/test_catXXX.py`
   - Rust: `asterix-rs/tests/test_catXXX.rs`

---

## DO-278A Mapping

| DO-278A Objective | Evidence | Status |
|-------------------|----------|--------|
| 6.4.2: Test Cases for Requirements | This mapping report | ✅ Documented |
| 6.4.3: Test Coverage | Per-category coverage | ⚠️ {round(cats_with_tests/total_cats*100, 1)}% |
| 6.4.4: Test Procedures | install/test/test.sh | ✅ Exists |

---

## Document Approval

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Test Engineer | TBD | {today} | |
| QA Manager | TBD | | |

---

## References

1. Low_Level_Requirements_CAT*.md (24 documents)
2. tests/cpp/test_*.cpp (C++ unit tests)
3. install/test/test.sh (Integration tests)
4. asterix/test/test_*.py (Python tests)
5. asterix-rs/tests/*.rs (Rust tests)
"""

    with open(output_path, 'w') as f:
        f.write(report)

    return coverage

def main():
    """Main function"""
    print("=" * 80)
    print("LLR TO TEST MAPPING TOOL - DO-278A Compliance")
    print("=" * 80)
    print()

    # Extract requirements
    print("1. Extracting LLR requirements...")
    requirements = extract_llr_requirements("do-278/requirements")
    print(f"   Found {len(requirements)} categories with {sum(len(r) for r in requirements.values())} requirements")

    # Find test files
    print()
    print("2. Finding test files...")
    test_files = find_test_files()
    total_test_files = sum(len(f) for f in test_files.values())
    print(f"   Found {total_test_files} test files:")
    for suite, files in test_files.items():
        print(f"   - {suite}: {len(files)} files")

    # Analyze coverage
    print()
    print("3. Analyzing test coverage...")
    coverage = analyze_test_coverage(requirements, test_files)

    # Generate report
    print()
    print("4. Generating report...")
    output_path = "do-278/requirements/LLR_Test_Mapping_Report.md"
    generate_test_mapping_report(coverage, output_path)

    # Summary
    cats_with_tests = sum(1 for c in coverage.values() if c['test_count'] > 0)
    print()
    print("=" * 80)
    print(f"✅ Analysis Complete!")
    print()
    print(f"Categories with tests: {cats_with_tests}/{len(coverage)}")
    print(f"Report generated: {output_path}")
    print("=" * 80)

if __name__ == "__main__":
    main()
