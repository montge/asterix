#!/usr/bin/env python3
"""
Performance comparison script for ASTERIX parsers (Rust vs Python vs C++)

Measures parsing performance across different implementations and data sizes.
Generates detailed benchmark reports.

Usage:
    python scripts/compare_performance.py [--iterations N] [--output report.md]
"""

import argparse
import os
import subprocess
import sys
import time
import json
from pathlib import Path
from typing import List, Dict, Tuple
import statistics

# Add parent directory to path to import asterix module
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

try:
    import asterix
    PYTHON_AVAILABLE = True
except ImportError:
    PYTHON_AVAILABLE = False
    print("⚠ Python asterix module not available")


class BenchmarkResult:
    """Stores benchmark results for a single test"""
    def __init__(self, name: str, language: str):
        self.name = name
        self.language = language
        self.times: List[float] = []
        self.record_count = 0
        self.data_size = 0

    def add_time(self, time_seconds: float):
        self.times.append(time_seconds)

    @property
    def mean_time(self) -> float:
        return statistics.mean(self.times) if self.times else 0.0

    @property
    def std_dev(self) -> float:
        return statistics.stdev(self.times) if len(self.times) > 1 else 0.0

    @property
    def throughput_mbps(self) -> float:
        """Throughput in MB/s"""
        if self.mean_time > 0:
            return (self.data_size / 1_000_000) / self.mean_time
        return 0.0

    @property
    def records_per_sec(self) -> float:
        """Records parsed per second"""
        if self.mean_time > 0:
            return self.record_count / self.mean_time
        return 0.0


def find_sample_data_dir() -> Path:
    """Find the sample data directory"""
    script_dir = Path(__file__).parent
    sample_dir = script_dir.parent.parent / "install" / "sample_data"

    if not sample_dir.exists():
        raise FileNotFoundError(f"Sample data directory not found: {sample_dir}")

    return sample_dir


def benchmark_python(data_path: Path, iterations: int) -> BenchmarkResult:
    """Benchmark Python asterix parser"""
    if not PYTHON_AVAILABLE:
        return None

    result = BenchmarkResult(data_path.name, "Python")

    # Read data
    with open(data_path, 'rb') as f:
        data = f.read()

    result.data_size = len(data)

    # Warmup
    asterix.parse(data)

    # Benchmark
    for _ in range(iterations):
        start = time.perf_counter()
        records = asterix.parse(data)
        elapsed = time.perf_counter() - start

        result.add_time(elapsed)
        if result.record_count == 0:
            result.record_count = len(records)

    return result


def benchmark_rust(data_path: Path, iterations: int, rust_project_dir: Path) -> BenchmarkResult:
    """Benchmark Rust parser using criterion benchmarks"""
    # This is a simplified version - actual benchmarks run via cargo bench
    # For now, we'll use a simple timing script

    result = BenchmarkResult(data_path.name, "Rust")
    result.data_size = data_path.stat().st_size

    # Check if Rust binary exists
    rust_bin = rust_project_dir / "target" / "release" / "examples" / "parse_pcap"

    if not rust_bin.exists():
        # Try to build it
        print(f"  Building Rust parser...")
        subprocess.run(
            ["cargo", "build", "--release", "--example", "parse_pcap"],
            cwd=rust_project_dir,
            check=True,
            capture_output=True
        )

    # Run benchmark
    for _ in range(iterations):
        start = time.perf_counter()
        proc = subprocess.run(
            [str(rust_bin), str(data_path)],
            capture_output=True,
            text=True
        )
        elapsed = time.perf_counter() - start

        if proc.returncode != 0:
            print(f"  ⚠ Rust parser failed: {proc.stderr}")
            return None

        result.add_time(elapsed)

        # Parse output to get record count
        if result.record_count == 0:
            for line in proc.stdout.split('\n'):
                if 'parsed' in line.lower() and 'record' in line.lower():
                    # Extract number from output
                    import re
                    match = re.search(r'(\d+)\s+record', line)
                    if match:
                        result.record_count = int(match.group(1))
                        break

    return result


def benchmark_cpp(data_path: Path, iterations: int) -> BenchmarkResult:
    """Benchmark C++ parser"""
    result = BenchmarkResult(data_path.name, "C++")
    result.data_size = data_path.stat().st_size

    # Find C++ executable
    cpp_bin = Path(__file__).parent.parent.parent / "install" / "bin" / "asterix"

    if not cpp_bin.exists():
        print(f"  ⚠ C++ parser not found at {cpp_bin}")
        return None

    # Read data to get record count
    with open(data_path, 'rb') as f:
        data = f.read()
    result.data_size = len(data)

    # Run benchmark
    for _ in range(iterations):
        start = time.perf_counter()
        proc = subprocess.run(
            [str(cpp_bin), "-f", str(data_path)],
            capture_output=True,
            text=True
        )
        elapsed = time.perf_counter() - start

        if proc.returncode != 0:
            print(f"  ⚠ C++ parser failed")
            return None

        result.add_time(elapsed)

    return result


def generate_report(results: Dict[str, List[BenchmarkResult]], output_path: Path = None):
    """Generate benchmark report in Markdown format"""
    lines = []
    lines.append("# ASTERIX Parser Performance Comparison")
    lines.append("")
    lines.append(f"**Generated:** {time.strftime('%Y-%m-%d %H:%M:%S')}")
    lines.append("")
    lines.append("## Summary")
    lines.append("")

    # Table header
    lines.append("| Test File | Language | Mean Time (ms) | Std Dev (ms) | Throughput (MB/s) | Records/sec | Speedup vs Python |")
    lines.append("|-----------|----------|----------------|--------------|-------------------|-------------|-------------------|")

    # Process results
    for test_name, test_results in results.items():
        python_result = next((r for r in test_results if r and r.language == "Python"), None)

        for result in test_results:
            if result is None:
                continue

            mean_ms = result.mean_time * 1000
            std_ms = result.std_dev * 1000

            # Calculate speedup vs Python
            speedup = ""
            if python_result and python_result.mean_time > 0 and result.language != "Python":
                speedup = f"{python_result.mean_time / result.mean_time:.2f}x"

            lines.append(f"| {test_name} | {result.language} | "
                        f"{mean_ms:.2f} | {std_ms:.2f} | "
                        f"{result.throughput_mbps:.2f} | "
                        f"{result.records_per_sec:.0f} | {speedup} |")

    lines.append("")
    lines.append("## Details")
    lines.append("")

    for test_name, test_results in results.items():
        lines.append(f"### {test_name}")
        lines.append("")

        for result in test_results:
            if result is None:
                continue

            lines.append(f"**{result.language}:**")
            lines.append(f"- Mean: {result.mean_time * 1000:.2f} ms")
            lines.append(f"- Std Dev: {result.std_dev * 1000:.2f} ms")
            lines.append(f"- Throughput: {result.throughput_mbps:.2f} MB/s")
            lines.append(f"- Records: {result.record_count}")
            lines.append(f"- Records/sec: {result.records_per_sec:.0f}")
            lines.append("")

    report = "\n".join(lines)

    if output_path:
        output_path.write_text(report)
        print(f"\n✓ Report written to: {output_path}")
    else:
        print("\n" + report)


def main():
    parser = argparse.ArgumentParser(description="Compare ASTERIX parser performance")
    parser.add_argument("--iterations", type=int, default=10,
                       help="Number of iterations per benchmark (default: 10)")
    parser.add_argument("--output", type=Path,
                       help="Output file for report (default: stdout)")
    parser.add_argument("--rust-only", action="store_true",
                       help="Only benchmark Rust (skip Python/C++)")
    parser.add_argument("--python-only", action="store_true",
                       help="Only benchmark Python (skip Rust/C++)")

    args = parser.parse_args()

    print("=== ASTERIX Parser Performance Comparison ===\n")

    # Find sample data
    try:
        sample_dir = find_sample_data_dir()
    except FileNotFoundError as e:
        print(f"Error: {e}")
        sys.exit(1)

    # Select test files
    test_files = [
        "cat048.raw",
        "cat_062_065.pcap",
        "cat_034_048.pcap",
    ]

    rust_dir = Path(__file__).parent.parent

    results = {}

    for test_file in test_files:
        data_path = sample_dir / test_file

        if not data_path.exists():
            print(f"⚠ Skipping {test_file} (not found)")
            continue

        print(f"\nBenchmarking: {test_file} ({data_path.stat().st_size} bytes)")
        print(f"  Iterations: {args.iterations}")

        test_results = []

        # Python benchmark
        if not args.rust_only and PYTHON_AVAILABLE:
            print(f"  Running Python benchmark...")
            python_result = benchmark_python(data_path, args.iterations)
            if python_result:
                test_results.append(python_result)
                print(f"    ✓ Mean: {python_result.mean_time * 1000:.2f} ms")

        # Rust benchmark
        if not args.python_only:
            print(f"  Running Rust benchmark...")
            rust_result = benchmark_rust(data_path, args.iterations, rust_dir)
            if rust_result:
                test_results.append(rust_result)
                print(f"    ✓ Mean: {rust_result.mean_time * 1000:.2f} ms")

        # C++ benchmark (skipped if Python-only or Rust-only)
        if not args.rust_only and not args.python_only:
            print(f"  Running C++ benchmark...")
            cpp_result = benchmark_cpp(data_path, args.iterations)
            if cpp_result:
                test_results.append(cpp_result)
                print(f"    ✓ Mean: {cpp_result.mean_time * 1000:.2f} ms")

        results[test_file] = test_results

    # Generate report
    generate_report(results, args.output)

    print("\n✓ Benchmark completed")


if __name__ == "__main__":
    main()
