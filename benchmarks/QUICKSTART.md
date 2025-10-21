# ASTERIX Benchmark Framework - Quick Start Guide

This guide will help you get started with benchmarking the ASTERIX decoder in under 5 minutes.

## 1. Build the Benchmarks

Choose one of the following methods:

### Option A: Using Make (Recommended for Quick Start)

```bash
cd benchmarks
make
```

### Option B: Using CMake

```bash
cd benchmarks
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cd ..
```

## 2. Run a Quick Test

Test that everything works:

```bash
# Run JSON output benchmark (fastest)
./build/bin/benchmark_json_output --records 1000 --iterations 2 --warmup 1

# Run PCAP processing benchmark (requires test data)
./data/generate_test_data.sh --size small
./build/bin/benchmark_pcap_processing --iterations 2 ./data/synthetic_small.raw
```

## 3. Run All Benchmarks

Use the automated runner:

```bash
./run_benchmarks.sh
```

This will:
- Build all benchmarks if needed
- Generate test data if not present
- Run all benchmark suites
- Generate a summary report in `results/latest/`

## 4. View Results

```bash
# View summary
cat results/latest/summary.txt

# View detailed JSON results
cat results/latest/benchmark_json_output.json | jq
```

## 5. Customize Your Benchmark Run

### More Iterations for Statistical Confidence

```bash
./run_benchmarks.sh --iterations 10 --warmup 2
```

### Verbose Output

```bash
./run_benchmarks.sh --verbose
```

### Quick Test (for development)

```bash
make test
# Or manually:
./run_benchmarks.sh --iterations 2 --warmup 1 --duration 5
```

## Common Use Cases

### Validate Performance After Changes

```bash
# Before your changes
./run_benchmarks.sh
cp -r results/latest results/baseline

# Make your changes...

# After your changes
./run_benchmarks.sh

# Compare
diff results/baseline/summary.txt results/latest/summary.txt
```

### CI/CD Integration

```bash
# Run in CI mode with threshold checking
./run_benchmarks.sh --ci --threshold 0.95

# Exit code:
#   0 = All benchmarks passed
#   1 = One or more benchmarks below threshold
```

### Individual Benchmark Runs

```bash
# JSON output only
./build/bin/benchmark_json_output \
  --records 100000 \
  --iterations 5 \
  --output results/json_test.json

# PCAP processing only
./build/bin/benchmark_pcap_processing \
  --iterations 5 \
  --format json \
  ../asterix/sample_data/cat_034_048.pcap

# UDP multicast only
./build/bin/benchmark_udp_multicast \
  --duration 30 \
  --rate 10000 \
  --output results/udp_test.json
```

## Troubleshooting

### "No such file or directory" when running benchmarks

Make sure you built the benchmarks first:
```bash
make clean
make
```

### "No sample data found" warnings

Generate synthetic test data:
```bash
cd data
./generate_test_data.sh --size medium
```

### Benchmark results are too variable

1. Close other applications
2. Disable CPU frequency scaling:
   ```bash
   sudo cpupower frequency-set --governor performance
   ```
3. Run more iterations:
   ```bash
   ./run_benchmarks.sh --iterations 20
   ```

### UDP benchmark fails to bind

Check if another process is using the port:
```bash
sudo netstat -tulpn | grep 21112
```

Change the port if needed:
```bash
./build/bin/benchmark_udp_multicast --port 21113
```

## Next Steps

- Read the full [README.md](README.md) for detailed documentation
- Check benchmark configurations in individual `.cpp` files
- Customize test data in `data/generate_test_data.sh`
- Integrate benchmarks into your CI/CD pipeline

## Performance Targets (Reference)

These are the target metrics from recent optimizations:

| Benchmark | Metric | Target |
|-----------|--------|--------|
| JSON Output | Records/sec | ≥50,000 |
| JSON Output | μs/record | ≤20 |
| PCAP Processing | Throughput | ≥1,500 MB/s |
| UDP Multicast | Packets/sec | ≥10,000 |
| UDP Multicast | Latency p95 | ≤100 μs |

If your benchmarks achieve these or better, the optimizations are working correctly!

## Support

- Documentation: [README.md](README.md)
- Issues: https://github.com/montge/asterix/issues
- Performance reports: See `../PERFORMANCE_*.md` files
