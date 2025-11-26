#!/usr/bin/env python3
"""
Mock Radar Performance Benchmarks

Measures radar simulation throughput.
"""

import sys
import time


def benchmark_plot_generation(num_iterations=1000):
    """Benchmark radar plot generation."""
    from asterix.radar_integration.mock_radar import MockRadar

    # Create radar at example location (London Heathrow area)
    radar = MockRadar(lat=51.47, lon=-0.45, alt=100.0, update_rate=1.0)

    # Warmup
    for _ in range(10):
        radar.generate_plots(num_targets=10)

    # Benchmark
    start = time.perf_counter()
    total_plots = 0
    for _ in range(num_iterations):
        plots = radar.generate_plots(num_targets=10)
        total_plots += len(plots)
    end = time.perf_counter()

    elapsed = end - start
    plots_per_sec = total_plots / elapsed

    print(f"Plot Generation Performance:")
    print(f"  Iterations: {num_iterations}")
    print(f"  Total plots: {total_plots}")
    print(f"  Time: {elapsed:.3f} seconds")
    print(f"  Throughput: {plots_per_sec:.0f} plots/second")

    return plots_per_sec


def benchmark_full_simulation(num_targets=50, duration_updates=100):
    """Benchmark full radar simulation."""
    from asterix.radar_integration.mock_radar import MockRadar

    # Create radar at example location
    radar = MockRadar(lat=51.47, lon=-0.45, alt=100.0, update_rate=10.0)

    start = time.perf_counter()
    total_plots = 0
    for _ in range(duration_updates):
        plots = radar.generate_plots(num_targets=num_targets)
        total_plots += len(plots)
    end = time.perf_counter()

    elapsed = end - start
    plots_per_sec = total_plots / elapsed
    simulated_time = duration_updates / 10.0  # 10 Hz update rate
    speedup = simulated_time / elapsed if elapsed > 0 else float('inf')

    print(f"\nFull Radar Simulation Performance:")
    print(f"  Targets: {num_targets}")
    print(f"  Simulation time: {simulated_time:.1f} seconds (simulated)")
    print(f"  Wall time: {elapsed:.3f} seconds")
    print(f"  Total plots: {total_plots}")
    print(f"  Throughput: {plots_per_sec:.0f} plots/second")
    print(f"  Speedup: {speedup:.2f}x realtime")

    return plots_per_sec, speedup


def main():
    try:
        from asterix.radar_integration.mock_radar import MockRadar
    except ImportError as e:
        print(f"Import error: {e}")
        print("Make sure to run: python setup.py install")
        return 1

    print("=" * 50)
    print("MOCK RADAR PERFORMANCE BENCHMARKS")
    print("=" * 50)
    print()

    plot_throughput = benchmark_plot_generation()
    sim_throughput, speedup = benchmark_full_simulation()

    print()
    print("=" * 50)
    print("SUMMARY")
    print("=" * 50)
    print(f"Plot generation: {plot_throughput:.0f} plots/sec")
    print(f"Full simulation: {sim_throughput:.0f} plots/sec ({speedup:.1f}x realtime)")

    # Save results for CI artifacts
    with open('mock_radar_benchmark_results.txt', 'w') as f:
        f.write(f"plot_generation_per_second={plot_throughput:.0f}\n")
        f.write(f"simulation_per_second={sim_throughput:.0f}\n")
        f.write(f"speedup_vs_realtime={speedup:.2f}\n")

    print("\n✅ Benchmarks completed successfully")
    return 0


if __name__ == "__main__":
    sys.exit(main())
