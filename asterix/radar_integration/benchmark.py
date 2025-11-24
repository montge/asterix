#!/usr/bin/env python3
"""
Performance Benchmark Suite for ASTERIX Radar Integration

Comprehensive benchmarks for:
- Encoder performance (all 6 categories)
- Mock radar generation speed
- Visualization rendering
- Statistics computation
- Decoder performance (when available)

Usage:
    python3 -m asterix.radar_integration.benchmark
    python3 -m asterix.radar_integration.benchmark --quick
    python3 -m asterix.radar_integration.benchmark --category CAT021
    python3 -m asterix.radar_integration.benchmark --save results.json

Results are saved and can be compared across versions to track performance.

License: GPL-3.0
"""

import time
import sys
import json
import argparse
from typing import Dict, List, Callable
from dataclasses import dataclass, asdict

from asterix.radar_integration import MockRadar
from asterix.radar_integration.encoder import (
    encode_cat001, encode_cat020, encode_cat021,
    encode_cat034_north_marker, encode_cat048, encode_cat062
)
from asterix.radar_integration.statistics import compute_scenario_stats


@dataclass
class BenchmarkResult:
    """Stores benchmark result for a single test."""
    name: str
    category: str
    iterations: int
    total_time: float
    avg_time: float
    throughput: float  # items/second
    unit: str
    details: Dict = None

    def to_dict(self):
        """Convert to dictionary for JSON serialization."""
        result = asdict(self)
        if result['details'] is None:
            result['details'] = {}
        return result


class BenchmarkSuite:
    """Performance benchmark suite."""

    def __init__(self, quick: bool = False):
        """
        Initialize benchmark suite.

        Args:
            quick: Run quick benchmarks (fewer iterations)
        """
        self.quick = quick
        self.results: List[BenchmarkResult] = []
        self.radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)

    def run_benchmark(
        self,
        name: str,
        category: str,
        func: Callable,
        iterations: int = None,
        unit: str = "ops"
    ) -> BenchmarkResult:
        """
        Run a single benchmark.

        Args:
            name: Benchmark name
            category: Category (e.g., "Encoder", "Generator")
            func: Function to benchmark
            iterations: Number of iterations (auto if None)
            unit: Unit for throughput (ops, plots, bytes, etc.)

        Returns:
            BenchmarkResult
        """
        if iterations is None:
            iterations = 100 if self.quick else 1000

        # Warmup
        for _ in range(min(10, iterations // 10)):
            func()

        # Benchmark
        start = time.perf_counter()
        for _ in range(iterations):
            func()
        elapsed = time.perf_counter() - start

        avg_time = elapsed / iterations
        throughput = iterations / elapsed if elapsed > 0 else 0

        result = BenchmarkResult(
            name=name,
            category=category,
            iterations=iterations,
            total_time=elapsed,
            avg_time=avg_time,
            throughput=throughput,
            unit=unit
        )

        self.results.append(result)
        return result

    def benchmark_mock_radar_generation(self):
        """Benchmark mock radar plot generation."""
        print("Benchmarking Mock Radar Generation...")

        # Small dataset
        result = self.run_benchmark(
            "Generate 10 plots",
            "Mock Radar",
            lambda: self.radar.generate_plots(10),
            iterations=1000,
            unit="generations/s"
        )
        print(f"  10 plots: {result.throughput:.0f} generations/s ({result.avg_time*1000:.2f}ms)")

        # Medium dataset
        result = self.run_benchmark(
            "Generate 100 plots",
            "Mock Radar",
            lambda: self.radar.generate_plots(100),
            iterations=500 if not self.quick else 50,
            unit="generations/s"
        )
        print(f"  100 plots: {result.throughput:.0f} generations/s ({result.avg_time*1000:.1f}ms)")

        # Large dataset
        result = self.run_benchmark(
            "Generate 1000 plots",
            "Mock Radar",
            lambda: self.radar.generate_plots(1000),
            iterations=100 if not self.quick else 10,
            unit="generations/s"
        )
        print(f"  1000 plots: {result.throughput:.1f} generations/s ({result.avg_time*1000:.0f}ms)")

        # Track generation
        result = self.run_benchmark(
            "Generate track (60s)",
            "Mock Radar",
            lambda: self.radar.generate_track(50e3, 45.0, 200.0, 90.0, 60.0),
            iterations=500 if not self.quick else 50,
            unit="tracks/s"
        )
        print(f"  Track: {result.throughput:.0f} tracks/s ({result.avg_time*1000:.2f}ms)")

    def benchmark_encoders(self):
        """Benchmark all ASTERIX encoders."""
        print("\nBenchmarking ASTERIX Encoders...")

        # Generate test data once
        plots_10 = self.radar.generate_plots(10)
        plots_100 = self.radar.generate_plots(100)

        tracks = [{'lat': 52.5 + i*0.01, 'lon': 13.4 + i*0.01, 'altitude_ft': 35000,
                   'vx': 100.0, 'vy': 200.0} for i in range(10)]

        adsb = [{'lat': 52.5 + i*0.01, 'lon': 13.4 + i*0.01,
                 'aircraft_address': 0x400000 + i, 'callsign': f'TST{i:03d}',
                 'flight_level': 350} for i in range(10)]

        # CAT001
        result = self.run_benchmark(
            "CAT001 (10 plots)",
            "Encoder",
            lambda: encode_cat001(plots_10, sac=0, sic=1),
            unit="encodings/s"
        )
        print(f"  CAT001: {result.throughput:.0f} encodings/s ({result.avg_time*1000:.2f}ms)")

        # CAT020
        result = self.run_benchmark(
            "CAT020 (10 targets)",
            "Encoder",
            lambda: encode_cat020(tracks, sac=0, sic=1),
            unit="encodings/s"
        )
        print(f"  CAT020: {result.throughput:.0f} encodings/s ({result.avg_time*1000:.2f}ms)")

        # CAT021
        result = self.run_benchmark(
            "CAT021 (10 reports)",
            "Encoder",
            lambda: encode_cat021(adsb, sac=0, sic=1),
            unit="encodings/s"
        )
        print(f"  CAT021: {result.throughput:.0f} encodings/s ({result.avg_time*1000:.2f}ms)")

        # CAT034
        result = self.run_benchmark(
            "CAT034 North Marker",
            "Encoder",
            lambda: encode_cat034_north_marker(sac=0, sic=1),
            unit="encodings/s"
        )
        print(f"  CAT034: {result.throughput:.0f} encodings/s ({result.avg_time*1000:.3f}ms)")

        # CAT048
        result = self.run_benchmark(
            "CAT048 (10 plots)",
            "Encoder",
            lambda: encode_cat048(plots_10, sac=0, sic=1),
            unit="encodings/s"
        )
        print(f"  CAT048: {result.throughput:.0f} encodings/s ({result.avg_time*1000:.2f}ms)")

        # CAT062
        result = self.run_benchmark(
            "CAT062 (10 tracks)",
            "Encoder",
            lambda: encode_cat062(tracks, sac=0, sic=1),
            unit="encodings/s"
        )
        print(f"  CAT062: {result.throughput:.0f} encodings/s ({result.avg_time*1000:.2f}ms)")

        # Large dataset
        result = self.run_benchmark(
            "CAT048 (100 plots)",
            "Encoder",
            lambda: encode_cat048(plots_100, sac=0, sic=1),
            iterations=500 if not self.quick else 50,
            unit="encodings/s"
        )
        bytes_per_encoding = len(encode_cat048(plots_100))
        throughput_mbps = (result.throughput * bytes_per_encoding) / (1024 * 1024)
        print(f"  CAT048 (100): {result.throughput:.0f} enc/s, {throughput_mbps:.2f} MB/s")

    def benchmark_statistics(self):
        """Benchmark statistics computation."""
        print("\nBenchmarking Statistics...")

        plots = self.radar.generate_plots(100)

        result = self.run_benchmark(
            "Scenario stats (100 plots)",
            "Statistics",
            lambda: compute_scenario_stats(plots),
            iterations=500 if not self.quick else 50,
            unit="computations/s"
        )
        print(f"  Scenario stats: {result.throughput:.0f} comp/s ({result.avg_time*1000:.2f}ms)")

    def benchmark_visualization(self):
        """Benchmark visualization rendering."""
        print("\nBenchmarking Visualization...")

        from asterix.radar_integration.visualization import plot_radar_ascii

        plots_10 = self.radar.generate_plots(10)
        plots_100 = self.radar.generate_plots(100)

        # ASCII rendering (no matplotlib)
        result = self.run_benchmark(
            "ASCII radar (10 plots)",
            "Visualization",
            lambda: plot_radar_ascii(plots_10),
            iterations=500 if not self.quick else 50,
            unit="renders/s"
        )
        print(f"  ASCII (10): {result.throughput:.0f} renders/s ({result.avg_time*1000:.2f}ms)")

        result = self.run_benchmark(
            "ASCII radar (100 plots)",
            "Visualization",
            lambda: plot_radar_ascii(plots_100),
            iterations=200 if not self.quick else 20,
            unit="renders/s"
        )
        print(f"  ASCII (100): {result.throughput:.0f} renders/s ({result.avg_time*1000:.1f}ms)")

    def run_all(self):
        """Run all benchmarks."""
        print("=" * 80)
        print("ASTERIX Radar Integration - Performance Benchmark Suite")
        print("=" * 80)
        print()

        self.benchmark_mock_radar_generation()
        self.benchmark_encoders()
        self.benchmark_statistics()
        self.benchmark_visualization()

        print()
        print("=" * 80)
        print(f"✅ Benchmark Complete - {len(self.results)} tests run")
        print("=" * 80)

    def print_summary(self):
        """Print summary table."""
        print()
        print("Summary by Category:")
        print("-" * 80)

        categories = {}
        for result in self.results:
            if result.category not in categories:
                categories[result.category] = []
            categories[result.category].append(result)

        for category, results in categories.items():
            print(f"\n{category}:")
            for r in results:
                print(f"  {r.name:40s} {r.throughput:10.1f} {r.unit}")

    def save_results(self, filename: str):
        """Save results to JSON file."""
        data = {
            'timestamp': time.time(),
            'quick_mode': self.quick,
            'total_tests': len(self.results),
            'results': [r.to_dict() for r in self.results]
        }

        with open(filename, 'w') as f:
            json.dump(data, f, indent=2)

        print(f"\n✅ Results saved to {filename}")


def main():
    """Run benchmark suite."""
    parser = argparse.ArgumentParser(description='ASTERIX Radar Integration Benchmarks')
    parser.add_argument('--quick', action='store_true', help='Run quick benchmarks (fewer iterations)')
    parser.add_argument('--category', choices=['all', 'mock', 'encoder', 'stats', 'viz'],
                        default='all', help='Benchmark category to run')
    parser.add_argument('--save', metavar='FILE', help='Save results to JSON file')

    args = parser.parse_args()

    suite = BenchmarkSuite(quick=args.quick)

    if args.category == 'all':
        suite.run_all()
    elif args.category == 'mock':
        suite.benchmark_mock_radar_generation()
    elif args.category == 'encoder':
        suite.benchmark_encoders()
    elif args.category == 'stats':
        suite.benchmark_statistics()
    elif args.category == 'viz':
        suite.benchmark_visualization()

    suite.print_summary()

    if args.save:
        suite.save_results(args.save)


if __name__ == "__main__":
    main()
