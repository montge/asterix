#!/usr/bin/env python3
"""
PCAP Validation Framework for ASTERIX Parser

This tool validates ASTERIX PCAP parsing by:
1. Running the parser on test PCAP files
2. Comparing output against golden reference files
3. Validating specific field values
4. Measuring performance (throughput)

Usage:
    python pcap_validator.py --all           # Run all tests
    python pcap_validator.py --golden        # Run golden file comparisons
    python pcap_validator.py --performance   # Run performance tests
    python pcap_validator.py --generate      # Generate golden files

Relates to GitHub Issue #41
"""

import argparse
import hashlib
import json
import os
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional, Tuple


@dataclass
class TestResult:
    """Result of a single test case."""
    name: str
    passed: bool
    message: str
    duration_ms: float = 0.0
    details: Optional[Dict] = None


@dataclass
class TestCase:
    """Definition of a test case."""
    name: str
    pcap_file: str
    format_flag: str  # -P, -R, -G, etc.
    output_format: str  # -j, -jh, -x, -xh, -l, or empty for text
    filter_file: Optional[str] = None
    expected_categories: Optional[List[int]] = None
    expected_record_count: Optional[int] = None
    golden_file: Optional[str] = None


class PCAPValidator:
    """PCAP validation framework for ASTERIX parser."""

    def __init__(self, repo_root: Path):
        self.repo_root = repo_root
        self.install_dir = repo_root / "install"
        self.test_dir = repo_root / "tests" / "pcap"
        self.data_dir = self.test_dir / "data"
        self.golden_dir = self.test_dir / "golden"
        self.sample_dir = self.install_dir / "share" / "asterix" / "samples"

        # Find executable
        self.executable = self.install_dir / "bin" / "asterix"
        self.config_file = self.install_dir / "config" / "asterix.ini"

        # Set library path for dynamic linking
        lib_path = self.install_dir / "lib"
        current_path = os.environ.get("LD_LIBRARY_PATH", "")
        os.environ["LD_LIBRARY_PATH"] = f"{lib_path}:{current_path}"

        # Define test cases
        self.test_cases = self._define_test_cases()

    def _define_test_cases(self) -> List[TestCase]:
        """Define all test cases for PCAP validation."""
        return [
            # Standard PCAP tests (from existing test.sh)
            TestCase(
                name="cat_062_065_json",
                pcap_file="cat_062_065.pcap",
                format_flag="-P",
                output_format="-j",
                expected_categories=[62, 65],
                golden_file="cat_062_065_json.golden"
            ),
            TestCase(
                name="cat_001_002_json_filtered",
                pcap_file="cat_001_002.pcap",
                format_flag="-R",
                output_format="-j",
                filter_file="filter.txt",
                expected_categories=[1, 2],
                golden_file="cat_001_002_json.golden"
            ),
            TestCase(
                name="cat_001_002_jsonh_filtered",
                pcap_file="cat_001_002.pcap",
                format_flag="-R",
                output_format="-jh",
                filter_file="filter.txt",
                expected_categories=[1, 2],
                golden_file="cat_001_002_jsonh.golden"
            ),
            TestCase(
                name="cat_001_002_xml_filtered",
                pcap_file="cat_001_002.pcap",
                format_flag="-R",
                output_format="-x",
                filter_file="filter.txt",
                expected_categories=[1, 2],
                golden_file="cat_001_002_xml.golden"
            ),
            TestCase(
                name="cat_001_002_text_filtered",
                pcap_file="cat_001_002.pcap",
                format_flag="-R",
                output_format="",
                filter_file="filter.txt",
                expected_categories=[1, 2],
                golden_file="cat_001_002_text.golden"
            ),
            TestCase(
                name="cat_001_002_line_filtered",
                pcap_file="cat_001_002.pcap",
                format_flag="-R",
                output_format="-l",
                filter_file="filter.txt",
                expected_categories=[1, 2],
                golden_file="cat_001_002_line.golden"
            ),
            TestCase(
                name="cat_034_048_filtered",
                pcap_file="cat_034_048.pcap",
                format_flag="-P",
                output_format="",
                filter_file="filter.txt",
                expected_categories=[34, 48],
                golden_file="cat_034_048_filtered.golden"
            ),
            TestCase(
                name="cat_034_048_unfiltered",
                pcap_file="cat_034_048.pcap",
                format_flag="-P",
                output_format="",
                expected_categories=[34, 48],
                golden_file="cat_034_048_unfiltered.golden"
            ),
            # JSON extensive output tests
            TestCase(
                name="cat_034_048_json_extensive",
                pcap_file="cat_034_048.pcap",
                format_flag="-P",
                output_format="-je",
                expected_categories=[34, 48],
                golden_file="cat_034_048_json_extensive.golden"
            ),
            # Multi-category mixed PCAP
            TestCase(
                name="asterix_mixed_json",
                pcap_file="asterix.pcap",
                format_flag="-P",
                output_format="-j",
                golden_file="asterix_mixed_json.golden"
            ),
        ]

    def run_parser(self, test_case: TestCase) -> Tuple[str, float, int]:
        """
        Run the ASTERIX parser on a test case.

        Returns:
            Tuple of (output, duration_ms, return_code)
        """
        # Find the PCAP file
        pcap_path = self.sample_dir / test_case.pcap_file
        if not pcap_path.exists():
            pcap_path = self.data_dir / test_case.pcap_file

        if not pcap_path.exists():
            raise FileNotFoundError(f"PCAP file not found: {test_case.pcap_file}")

        # Build command
        cmd = [
            str(self.executable),
            "-d", str(self.config_file),
            test_case.format_flag,
            "-f", str(pcap_path)
        ]

        # Add output format flag if specified
        if test_case.output_format:
            cmd.append(test_case.output_format)

        # Add filter file if specified
        if test_case.filter_file:
            filter_path = self.install_dir / "sample_output" / test_case.filter_file
            if filter_path.exists():
                cmd.extend(["-LF", str(filter_path)])

        # Run parser and measure time
        start_time = time.perf_counter()
        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=60  # 60 second timeout
            )
            duration_ms = (time.perf_counter() - start_time) * 1000
            return result.stdout, duration_ms, result.returncode
        except subprocess.TimeoutExpired:
            return "", 60000, -1

    def compare_with_golden(self, test_case: TestCase, output: str) -> TestResult:
        """Compare parser output with golden reference file."""
        golden_path = self.golden_dir / test_case.golden_file

        if not golden_path.exists():
            return TestResult(
                name=test_case.name,
                passed=False,
                message=f"Golden file not found: {golden_path}"
            )

        with open(golden_path, "r") as f:
            golden_content = f.read()

        # Normalize both outputs: strip trailing whitespace from each line
        def normalize(text):
            return '\n'.join(line.rstrip() for line in text.split('\n')).strip()

        if normalize(output) == normalize(golden_content):
            return TestResult(
                name=test_case.name,
                passed=True,
                message="Output matches golden file"
            )
        else:
            # Find first difference (using normalized lines)
            output_lines = [line.rstrip() for line in output.strip().split('\n')]
            golden_lines = [line.rstrip() for line in golden_content.strip().split('\n')]

            diff_line = 0
            for i, (out, gold) in enumerate(zip(output_lines, golden_lines)):
                if out != gold:
                    diff_line = i + 1
                    break
            else:
                diff_line = min(len(output_lines), len(golden_lines)) + 1

            return TestResult(
                name=test_case.name,
                passed=False,
                message=f"Output differs from golden file at line {diff_line}",
                details={
                    "output_lines": len(output_lines),
                    "golden_lines": len(golden_lines),
                    "first_diff_line": diff_line
                }
            )

    def validate_categories(self, test_case: TestCase, output: str) -> TestResult:
        """Validate that expected categories are present in output."""
        if not test_case.expected_categories:
            return TestResult(
                name=f"{test_case.name}_categories",
                passed=True,
                message="No category validation required"
            )

        found_categories = set()
        import re

        for line in output.split('\n'):
            # Text format: "Category: 48"
            if 'Category:' in line:
                try:
                    cat = int(line.split('Category:')[1].split()[0])
                    found_categories.add(cat)
                except (ValueError, IndexError):
                    pass

            # JSON format: "cat":48 or "cat": 48
            json_matches = re.findall(r'"cat"\s*:\s*(\d+)', line)
            for match in json_matches:
                found_categories.add(int(match))

            # XML format: <Category>48</Category> or cat="48"
            xml_matches = re.findall(r'<Category>(\d+)</Category>', line)
            for match in xml_matches:
                found_categories.add(int(match))
            xml_attr_matches = re.findall(r'cat="(\d+)"', line, re.IGNORECASE)
            for match in xml_attr_matches:
                found_categories.add(int(match))

            # Line format: Asterix.CAT048.xxx or Asterix.CAT001.xxx
            line_matches = re.findall(r'Asterix\.CAT(\d+)\.', line)
            for match in line_matches:
                found_categories.add(int(match))

        expected = set(test_case.expected_categories)
        if found_categories >= expected:
            return TestResult(
                name=f"{test_case.name}_categories",
                passed=True,
                message=f"Found expected categories: {sorted(found_categories)}"
            )
        else:
            missing = expected - found_categories
            return TestResult(
                name=f"{test_case.name}_categories",
                passed=False,
                message=f"Missing categories: {sorted(missing)}",
                details={
                    "expected": sorted(expected),
                    "found": sorted(found_categories),
                    "missing": sorted(missing)
                }
            )

    def generate_golden_file(self, test_case: TestCase) -> TestResult:
        """Generate a golden file from current parser output."""
        try:
            output, duration_ms, returncode = self.run_parser(test_case)

            if returncode != 0:
                return TestResult(
                    name=test_case.name,
                    passed=False,
                    message=f"Parser returned non-zero exit code: {returncode}"
                )

            if not output.strip():
                return TestResult(
                    name=test_case.name,
                    passed=False,
                    message="Parser produced no output"
                )

            golden_path = self.golden_dir / test_case.golden_file
            golden_path.parent.mkdir(parents=True, exist_ok=True)

            with open(golden_path, "w") as f:
                f.write(output)

            return TestResult(
                name=test_case.name,
                passed=True,
                message=f"Generated golden file: {golden_path}",
                duration_ms=duration_ms
            )
        except Exception as e:
            return TestResult(
                name=test_case.name,
                passed=False,
                message=f"Error generating golden file: {str(e)}"
            )

    def run_performance_test(self, test_case: TestCase, iterations: int = 5) -> TestResult:
        """Run performance test on a PCAP file."""
        pcap_path = self.sample_dir / test_case.pcap_file
        if not pcap_path.exists():
            pcap_path = self.data_dir / test_case.pcap_file

        if not pcap_path.exists():
            return TestResult(
                name=f"{test_case.name}_perf",
                passed=False,
                message=f"PCAP file not found: {test_case.pcap_file}"
            )

        file_size_kb = pcap_path.stat().st_size / 1024
        durations = []

        for _ in range(iterations):
            try:
                _, duration_ms, returncode = self.run_parser(test_case)
                if returncode == 0:
                    durations.append(duration_ms)
            except Exception:
                pass

        if not durations:
            return TestResult(
                name=f"{test_case.name}_perf",
                passed=False,
                message="All performance iterations failed"
            )

        avg_duration_ms = sum(durations) / len(durations)
        throughput_kbps = (file_size_kb / avg_duration_ms) * 1000

        return TestResult(
            name=f"{test_case.name}_perf",
            passed=True,
            message=f"Throughput: {throughput_kbps:.2f} KB/s",
            duration_ms=avg_duration_ms,
            details={
                "file_size_kb": file_size_kb,
                "avg_duration_ms": avg_duration_ms,
                "throughput_kbps": throughput_kbps,
                "iterations": len(durations)
            }
        )

    def run_all_tests(self) -> List[TestResult]:
        """Run all validation tests."""
        results = []

        for test_case in self.test_cases:
            if not test_case.golden_file:
                continue

            print(f"Running: {test_case.name}...", end=" ", flush=True)

            try:
                output, duration_ms, returncode = self.run_parser(test_case)

                if returncode != 0:
                    results.append(TestResult(
                        name=test_case.name,
                        passed=False,
                        message=f"Parser returned exit code {returncode}",
                        duration_ms=duration_ms
                    ))
                    print("FAILED (exit code)")
                    continue

                # Compare with golden file
                result = self.compare_with_golden(test_case, output)
                result.duration_ms = duration_ms
                results.append(result)

                # Validate categories
                cat_result = self.validate_categories(test_case, output)
                results.append(cat_result)

                print("OK" if result.passed else "FAILED")

            except FileNotFoundError as e:
                results.append(TestResult(
                    name=test_case.name,
                    passed=False,
                    message=str(e)
                ))
                print("SKIPPED (file not found)")
            except Exception as e:
                results.append(TestResult(
                    name=test_case.name,
                    passed=False,
                    message=f"Error: {str(e)}"
                ))
                print("ERROR")

        return results

    def run_golden_generation(self) -> List[TestResult]:
        """Generate all golden files."""
        results = []

        for test_case in self.test_cases:
            if not test_case.golden_file:
                continue

            print(f"Generating: {test_case.golden_file}...", end=" ", flush=True)
            result = self.generate_golden_file(test_case)
            results.append(result)
            print("OK" if result.passed else "FAILED")

        return results

    def run_performance_tests(self) -> List[TestResult]:
        """Run all performance tests."""
        results = []

        for test_case in self.test_cases:
            print(f"Performance: {test_case.name}...", end=" ", flush=True)
            result = self.run_performance_test(test_case)
            results.append(result)
            print(f"{result.details.get('throughput_kbps', 0):.2f} KB/s" if result.passed else "FAILED")

        return results


def print_summary(results: List[TestResult]) -> int:
    """Print test summary and return exit code."""
    passed = sum(1 for r in results if r.passed)
    failed = sum(1 for r in results if not r.passed)
    total = len(results)

    print("\n" + "=" * 60)
    print(f"PCAP Validation Summary")
    print("=" * 60)
    print(f"Passed: {passed}/{total}")
    print(f"Failed: {failed}/{total}")

    if failed > 0:
        print("\nFailed tests:")
        for r in results:
            if not r.passed:
                print(f"  - {r.name}: {r.message}")

    print("=" * 60)

    return 0 if failed == 0 else 1


def main():
    parser = argparse.ArgumentParser(description="PCAP Validation Framework for ASTERIX")
    parser.add_argument("--all", action="store_true", help="Run all tests")
    parser.add_argument("--golden", action="store_true", help="Run golden file comparisons")
    parser.add_argument("--generate", action="store_true", help="Generate golden files")
    parser.add_argument("--performance", action="store_true", help="Run performance tests")
    parser.add_argument("--repo-root", type=Path, default=Path(__file__).parent.parent.parent.parent,
                        help="Repository root directory")

    args = parser.parse_args()

    # Default to --all if no option specified
    if not any([args.all, args.golden, args.generate, args.performance]):
        args.all = True

    validator = PCAPValidator(args.repo_root)

    # Check prerequisites
    if not validator.executable.exists():
        print(f"Error: Executable not found at {validator.executable}")
        print("Please build and install the project first:")
        print("  cmake -B build && cmake --build build && cmake --install build")
        sys.exit(1)

    results = []

    if args.generate:
        print("Generating golden files...")
        results.extend(validator.run_golden_generation())

    if args.all or args.golden:
        print("\nRunning golden file comparisons...")
        results.extend(validator.run_all_tests())

    if args.performance:
        print("\nRunning performance tests...")
        results.extend(validator.run_performance_tests())

    return print_summary(results)


if __name__ == "__main__":
    sys.exit(main())
