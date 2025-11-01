# Rust Bindings Scripts

This directory contains utility scripts for testing, benchmarking, and comparing the Rust ASTERIX bindings.

## Available Scripts

### compare_performance.py

**Purpose:** Compare parsing performance between Rust, Python, and C++ implementations.

**Requirements:**
- Python 3.7+
- asterix Python module installed (`python setup.py install` from project root)
- Rust release build (`cargo build --release`)
- C++ executable at `install/bin/asterix`

**Usage:**

```bash
# Full comparison (Rust vs Python vs C++)
python scripts/compare_performance.py --iterations 100 --output benchmark_report.md

# Rust only
python scripts/compare_performance.py --rust-only --iterations 50

# Python only
python scripts/compare_performance.py --python-only --iterations 50

# Help
python scripts/compare_performance.py --help
```

**Options:**
- `--iterations N` - Number of iterations per benchmark (default: 10)
- `--output FILE` - Output file for Markdown report (default: stdout)
- `--rust-only` - Only benchmark Rust implementation
- `--python-only` - Only benchmark Python implementation

**Output Format:**

The script generates a Markdown report with:
- Summary table comparing all implementations
- Mean time, standard deviation, throughput
- Speedup calculations vs Python baseline
- Detailed statistics per test file

**Example Output:**

```markdown
# ASTERIX Parser Performance Comparison

| Test File | Language | Mean Time (ms) | Std Dev (ms) | Throughput (MB/s) | Speedup vs Python |
|-----------|----------|----------------|--------------|-------------------|-------------------|
| cat048.raw | Python | 0.12 | 0.01 | 0.40 | - |
| cat048.raw | Rust | 0.05 | 0.005 | 0.96 | 2.4x |
| cat048.raw | C++ | 0.048 | 0.004 | 1.00 | 2.5x |
```

**Test Files:**

The script automatically tests these files from `install/sample_data/`:
- `cat048.raw` (48 bytes) - Small raw data
- `cat_062_065.pcap` (255 bytes) - Medium PCAP
- `cat_034_048.pcap` (12.7 KB) - Large PCAP

## Adding New Scripts

When adding new scripts to this directory:

1. **Make executable:**
   ```bash
   chmod +x scripts/your_script.py
   ```

2. **Add shebang:**
   ```python
   #!/usr/bin/env python3
   ```

3. **Add documentation:**
   - Docstring at top of file
   - Usage examples
   - Update this README

4. **Follow conventions:**
   - Use argparse for command-line arguments
   - Provide `--help` option
   - Write output to stdout or file (user choice)
   - Return non-zero exit code on error

## Future Scripts

Potential scripts to add:

- **`validate_coverage.py`** - Check test coverage meets minimum threshold
- **`generate_fixtures.py`** - Generate test fixtures from live data
- **`profile_memory.py`** - Memory profiling with visualization
- **`check_dependencies.py`** - Verify all build dependencies installed
- **`update_docs.py`** - Auto-generate documentation from source

## Notes

- All scripts should work from project root directory
- Use relative paths for portability
- Test scripts on all supported platforms
- Document any external dependencies
