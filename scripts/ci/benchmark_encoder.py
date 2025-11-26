#!/usr/bin/env python3
"""
Encoder Performance Benchmarks

Measures encoding throughput (records per second).
"""

import sys
import time


def benchmark_single_encoding(iterations=10000):
    """Benchmark single record encoding."""
    from asterix.radar_integration.encoder.cat048 import encode_cat048_record

    # Warmup
    for _ in range(100):
        encode_cat048_record(range_m=50000.0, azimuth_deg=90.0, sac=10, sic=1)

    # Benchmark
    times = []
    for _ in range(10):  # 10 runs for statistics
        start = time.perf_counter()
        for _ in range(iterations):
            encode_cat048_record(
                range_m=50000.0,
                azimuth_deg=90.0,
                sac=10,
                sic=1,
                mode3a=0o1234
            )
        end = time.perf_counter()
        times.append(end - start)

    avg_time = sum(times) / len(times)
    std_time = (sum((t - avg_time) ** 2 for t in times) / len(times)) ** 0.5
    records_per_sec = iterations / avg_time

    print(f"CAT048 Single Record Encoding:")
    print(f"  Iterations: {iterations}")
    print(f"  Average time: {avg_time:.4f} seconds")
    print(f"  Std dev: {std_time:.4f} seconds")
    print(f"  Throughput: {records_per_sec:.0f} records/second")
    print(f"  Time per record: {(avg_time/iterations)*1000:.3f} ms")

    return records_per_sec


def benchmark_bulk_encoding(num_records=1000):
    """Benchmark bulk encoding (multiple records in one datablock)."""
    from asterix.radar_integration.encoder.cat048 import (
        encode_cat048_record,
        encode_cat048_datablock
    )

    # Pre-generate records
    records = []
    for i in range(num_records):
        records.append(encode_cat048_record(
            range_m=10000.0 + i * 10,
            azimuth_deg=(i * 0.36) % 360,
            sac=i % 256,
            sic=(i // 256) % 256,
            mode3a=i % 0o7777
        ))

    # Benchmark datablock creation
    start = time.perf_counter()
    datablock = encode_cat048_datablock(records)
    end = time.perf_counter()

    elapsed = end - start
    records_per_sec = num_records / elapsed
    mb_per_sec = (len(datablock) / (1024 * 1024)) / elapsed

    print(f"\nBulk Encoding Performance:")
    print(f"  Records: {num_records}")
    print(f"  Total bytes: {len(datablock)}")
    print(f"  Time: {elapsed:.3f} seconds")
    print(f"  Throughput: {records_per_sec:.0f} records/second")
    print(f"  Data rate: {mb_per_sec:.2f} MB/second")

    return records_per_sec


def main():
    try:
        from asterix.radar_integration.encoder.cat048 import (
            encode_cat048_record,
            encode_cat048_datablock
        )
    except ImportError as e:
        print(f"Import error: {e}")
        print("Make sure to run: python setup.py install")
        return 1

    print("=" * 50)
    print("ENCODER PERFORMANCE BENCHMARKS")
    print("=" * 50)
    print()

    single_throughput = benchmark_single_encoding()
    bulk_throughput = benchmark_bulk_encoding()

    print()
    print("=" * 50)
    print("SUMMARY")
    print("=" * 50)
    print(f"Single record throughput: {single_throughput:.0f} records/sec")
    print(f"Bulk encoding throughput: {bulk_throughput:.0f} records/sec")

    # Save results for CI artifacts
    with open('encoder_benchmark_results.txt', 'w') as f:
        f.write(f"single_records_per_second={single_throughput:.0f}\n")
        f.write(f"bulk_records_per_second={bulk_throughput:.0f}\n")

    print("\n✅ Benchmarks completed successfully")
    return 0


if __name__ == "__main__":
    sys.exit(main())
