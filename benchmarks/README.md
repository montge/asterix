# ASTERIX Performance Benchmark Framework

This directory contains a comprehensive performance benchmarking framework for validating and measuring the ASTERIX decoder's performance characteristics.

## Overview

The benchmark framework measures:
- **UDP Multicast Performance**: Packets/second throughput, processing latency, CPU usage
- **PCAP File Processing**: File processing time, throughput (MB/s), memory usage
- **JSON Output Generation**: Generation time, string allocation efficiency

## Purpose

This framework was created to:
1. Validate the 55-61% speedup claims from recent optimizations
2. Establish performance baselines for regression testing
3. Identify performance bottlenecks across different workloads
4. Enable continuous performance monitoring

## Directory Structure

```
benchmarks/
├── README.md                          # This file
├── CMakeLists.txt                     # Build configuration
├── run_benchmarks.sh                  # Main benchmark runner script
├── benchmark_udp_multicast.cpp        # UDP multicast throughput benchmark
├── benchmark_pcap_processing.cpp      # PCAP file processing benchmark
├── benchmark_json_output.cpp          # JSON generation benchmark
├── benchmark_common.h                 # Common utilities and timing functions
├── data/                              # Test data files
│   ├── generate_test_data.sh          # Script to generate synthetic test data
│   └── README.md                      # Data file documentation
└── results/                           # Benchmark results (gitignored)
    └── .gitkeep
```

## Building Benchmarks

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.12 or higher
- libexpat-devel
- (Optional) Google Benchmark library for advanced statistical analysis

### Build Instructions

#### Using CMake (Recommended)

```bash
# From the asterix root directory
cd benchmarks
mkdir build && cd build
cmake ..
make -j$(nproc)

# Binaries will be in benchmarks/build/
```

#### Using Make (Alternative)

```bash
# From the benchmarks directory
cd benchmarks
make

# Binaries will be in benchmarks/bin/
```

#### Build Options

- `BUILD_WITH_GOOGLE_BENCHMARK=ON`: Use Google Benchmark library (if available)
- `CMAKE_BUILD_TYPE=Release`: Always use Release builds for benchmarking
- `ENABLE_COVERAGE=OFF`: Disable coverage for accurate performance measurement

```bash
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_WITH_GOOGLE_BENCHMARK=ON ..
```

## Running Benchmarks

### Quick Start

Run all benchmarks with default settings:

```bash
./run_benchmarks.sh
```

This will:
1. Build the benchmarks (if needed)
2. Generate test data (if not present)
3. Run all benchmark suites
4. Generate summary report in `results/latest/`

### Individual Benchmarks

#### UDP Multicast Benchmark

```bash
./build/benchmark_udp_multicast [OPTIONS]

Options:
  --duration <seconds>       Duration to run benchmark (default: 10)
  --port <port>              UDP port to listen on (default: 21112)
  --packet-size <bytes>      Packet size to generate (default: 1024)
  --rate <pps>               Packets per second to generate (default: 1000)
  --output <file>            Output results to JSON file
  --baseline <commit>        Compare against baseline commit
```

Example:
```bash
# Benchmark 10 seconds of UDP multicast at 5000 pps
./build/benchmark_udp_multicast --duration 10 --rate 5000 --output results/udp_5kpps.json
```

#### PCAP Processing Benchmark

```bash
./build/benchmark_pcap_processing [OPTIONS] <pcap_file>

Options:
  --iterations <n>           Number of iterations (default: 5)
  --warmup <n>               Warmup iterations (default: 1)
  --output <file>            Output results to JSON file
  --format <fmt>             Output format: text|json|xml (default: json)
  --baseline <commit>        Compare against baseline commit
```

Example:
```bash
# Benchmark PCAP processing with JSON output
./build/benchmark_pcap_processing --iterations 10 --format json data/cat_034_048.pcap
```

#### JSON Output Benchmark

```bash
./build/benchmark_json_output [OPTIONS]

Options:
  --records <n>              Number of records to process (default: 10000)
  --iterations <n>           Number of iterations (default: 5)
  --warmup <n>               Warmup iterations (default: 1)
  --output <file>            Output results to JSON file
  --baseline <commit>        Compare against baseline commit
```

Example:
```bash
# Benchmark JSON generation with 100k records
./build/benchmark_json_output --records 100000 --iterations 5 --output results/json_100k.json
```

## Benchmark Metrics

### UDP Multicast Benchmark

**Primary Metrics:**
- **Throughput**: Packets per second (pps)
- **Data Rate**: Megabytes per second (MB/s)
- **Latency Percentiles**: p50, p95, p99, p99.9 (microseconds)
- **CPU Usage**: Percent of CPU time consumed
- **Memory**: Peak RSS, allocation count

**Secondary Metrics:**
- Packet loss rate
- Processing jitter (latency standard deviation)
- Context switches

### PCAP Processing Benchmark

**Primary Metrics:**
- **Processing Time**: Total time to process file (seconds)
- **Throughput**: Megabytes per second (MB/s)
- **Parse Rate**: Records per second
- **Memory Peak**: Maximum RSS during processing

**Secondary Metrics:**
- Parse error count
- Memory allocations per record
- Cache miss rate (if available)

### JSON Output Benchmark

**Primary Metrics:**
- **Generation Time**: Microseconds per record
- **Throughput**: Records per second
- **Memory Efficiency**: Bytes allocated per record
- **String Allocations**: Count per record

**Secondary Metrics:**
- JSON string length statistics
- Reallocation count
- Buffer utilization

## Interpreting Results

### Output Format

All benchmarks generate results in JSON format for machine readability:

```json
{
  "benchmark": "udp_multicast",
  "timestamp": "2025-10-20T22:30:00Z",
  "git_commit": "a769202",
  "build_config": {
    "compiler": "gcc 11.4.0",
    "optimization": "-O3",
    "cxx_standard": "17"
  },
  "system_info": {
    "cpu": "Intel Core i7-9700K",
    "cores": 8,
    "ram_gb": 32,
    "os": "Linux 6.6.87.2"
  },
  "metrics": {
    "throughput_pps": 12543.2,
    "throughput_mbps": 12.8,
    "latency_p50_us": 42.3,
    "latency_p95_us": 78.1,
    "latency_p99_us": 125.4,
    "cpu_percent": 23.5,
    "memory_peak_mb": 45.2
  }
}
```

### Baseline Comparison

To compare performance against a baseline:

```bash
# Run benchmark against baseline commit
./run_benchmarks.sh --baseline a769202

# Or for individual benchmarks
./build/benchmark_pcap_processing --baseline a769202 data/test.pcap
```

The script will:
1. Checkout the baseline commit
2. Build and run benchmarks
3. Checkout the current commit
4. Build and run benchmarks
5. Generate comparison report

Example output:
```
=== Performance Comparison ===
Baseline: a769202 (2025-10-19)
Current:  HEAD (2025-10-20)

PCAP Processing:
  Throughput:  1234.5 MB/s → 1876.3 MB/s  (+52.0% ✓)
  Latency p95:   125 μs   →    78 μs     (-37.6% ✓)
  Memory Peak:   48 MB   →    32 MB      (-33.3% ✓)
```

## Performance Targets

Based on the optimization work, we target:

### UDP Multicast
- **Throughput**: ≥10,000 pps for 1KB packets
- **Latency p95**: ≤100 μs
- **Latency p99**: ≤200 μs
- **CPU Usage**: ≤30% at 10,000 pps

### PCAP Processing
- **Throughput**: ≥1,500 MB/s for typical ASTERIX data
- **Memory Peak**: ≤50 MB for 100MB files
- **Parse Rate**: ≥50,000 records/sec

### JSON Output
- **Generation Time**: ≤20 μs per record
- **Memory Efficiency**: ≤2 KB allocated per record
- **Throughput**: ≥50,000 records/sec

## Regression Testing

To use benchmarks for continuous integration:

```bash
# Run benchmarks and check against thresholds
./run_benchmarks.sh --ci --threshold 0.95

# Exit code:
#   0 = All benchmarks meet thresholds
#   1 = One or more benchmarks below threshold
#   2 = Benchmark execution failed
```

CI configuration example (GitHub Actions):

```yaml
name: Performance Benchmarks

on: [pull_request]

jobs:
  benchmark:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install dependencies
        run: sudo apt-get install -y libexpat-dev
      - name: Run benchmarks
        run: |
          cd benchmarks
          ./run_benchmarks.sh --ci --threshold 0.95
      - name: Upload results
        uses: actions/upload-artifact@v3
        with:
          name: benchmark-results
          path: benchmarks/results/
```

## Troubleshooting

### Benchmark Results Too Variable

1. **Disable frequency scaling:**
   ```bash
   # Linux
   sudo cpupower frequency-set --governor performance
   ```

2. **Isolate CPU cores:**
   ```bash
   # Run benchmark on isolated core
   taskset -c 3 ./build/benchmark_pcap_processing data/test.pcap
   ```

3. **Increase iterations:**
   ```bash
   ./build/benchmark_pcap_processing --iterations 20 --warmup 5 data/test.pcap
   ```

### Out of Memory

For memory-constrained systems:

```bash
# Reduce test data size
./data/generate_test_data.sh --size small

# Limit record count
./build/benchmark_json_output --records 1000
```

### UDP Multicast Permissions

On Linux, you may need to increase buffer sizes:

```bash
# Increase UDP receive buffer
sudo sysctl -w net.core.rmem_max=26214400
sudo sysctl -w net.core.rmem_default=26214400
```

## Test Data Generation

Generate synthetic test data for benchmarks:

```bash
cd data
./generate_test_data.sh

# Generate specific sizes
./generate_test_data.sh --size large    # 1GB test file
./generate_test_data.sh --size medium   # 100MB test file
./generate_test_data.sh --size small    # 10MB test file
```

## Analyzing Results

### Viewing Results

```bash
# Latest results
cat results/latest/summary.json | jq

# Specific benchmark
cat results/latest/benchmark_pcap_processing.json | jq '.metrics'

# Historical comparison
python3 scripts/compare_results.py results/2025-10-19/ results/latest/
```

### Plotting Performance Over Time

```bash
# Generate performance trend graphs (requires matplotlib)
python3 scripts/plot_trends.py results/

# Output: results/trends/*.png
```

## Contributing

When adding new benchmarks:

1. Follow the existing structure in `benchmark_common.h`
2. Output results in JSON format
3. Include warmup iterations
4. Document metrics in this README
5. Add to `run_benchmarks.sh`

Example benchmark skeleton:

```cpp
#include "benchmark_common.h"

int main(int argc, char** argv) {
    BenchmarkConfig config = parse_args(argc, argv);
    BenchmarkResults results;

    // Warmup
    for (int i = 0; i < config.warmup_iterations; i++) {
        run_workload();
    }

    // Benchmark
    auto start = high_resolution_clock::now();
    for (int i = 0; i < config.iterations; i++) {
        auto iter_start = high_resolution_clock::now();
        run_workload();
        auto iter_end = high_resolution_clock::now();
        results.add_iteration(iter_start, iter_end);
    }
    auto end = high_resolution_clock::now();

    // Report
    results.print_summary();
    results.save_json(config.output_file);

    return 0;
}
```

## References

- [ASTERIX Protocol Specification](https://www.eurocontrol.int/asterix)
- [Google Benchmark Documentation](https://github.com/google/benchmark)
- [Performance Analysis Guide](../docs/performance.md)
- [Optimization Report](../PERFORMANCE_OPTIMIZATIONS_COMPLETED.md)

## Support

For questions or issues with benchmarks:
- GitHub Issues: https://github.com/montge/asterix/issues
- Documentation: ../docs/
- Performance Reports: ../PERFORMANCE_*.md
