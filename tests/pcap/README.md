# PCAP Validation and Testing Framework

This directory contains the PCAP validation framework for the ASTERIX parser, ensuring correct parsing across all bindings and the Wireshark plugin.

## Directory Structure

```
tests/pcap/
├── README.md           # This file
├── data/               # Test PCAP files (beyond samples)
├── golden/             # Golden reference outputs
└── tools/              # Validation tools
    └── pcap_validator.py
```

## Quick Start

```bash
# Run all tests
python tests/pcap/tools/pcap_validator.py --all

# Generate golden files (after parser changes)
python tests/pcap/tools/pcap_validator.py --generate

# Run performance tests
python tests/pcap/tools/pcap_validator.py --performance
```

## Test Cases

### Standard PCAP Tests

| Test Name | PCAP File | Categories | Description |
|-----------|-----------|------------|-------------|
| cat_062_065_json | cat_062_065.pcap | 62, 65 | SDPS track messages |
| cat_001_002_json_filtered | cat_001_002.pcap | 1, 2 | Monoradar with filter |
| cat_034_048_unfiltered | cat_034_048.pcap | 34, 48 | Radar service + target reports |
| asterix_mixed_json | asterix.pcap | Mixed | Multi-category mixed traffic |

### Output Formats Tested

- **Text** (default) - Human-readable output
- **JSON** (`-j`) - Compact JSON, one object per line
- **JSON Human** (`-jh`) - Pretty-printed JSON
- **JSON Extensive** (`-je`) - JSON with descriptions
- **XML** (`-x`) - Compact XML
- **Line** (`-l`) - One line per item

### Input Formats Tested

- **PCAP** (`-P`) - Standard PCAP encapsulation
- **ORADIS PCAP** (`-R`) - ORADIS variant
- **GPS** (`-G`) - GPS packet format

## Golden File Tests

Golden files are reference outputs used for regression testing. When the parser output changes, tests fail until golden files are regenerated.

### Regenerating Golden Files

**WARNING:** Only regenerate after verifying the new output is correct!

```bash
# Regenerate all golden files
python tests/pcap/tools/pcap_validator.py --generate

# Verify changes
git diff tests/pcap/golden/
```

## Performance Baselines

Performance tests measure parsing throughput in KB/s:

| Test | File Size | Baseline Throughput |
|------|-----------|---------------------|
| cat_034_048 | 12.5 KB | >1000 KB/s |
| cat_062_065 | 0.25 KB | >500 KB/s |
| asterix.pcap | 11.1 KB | >1000 KB/s |

## Adding New Test Cases

1. Add PCAP file to `data/` or use existing file from `install/share/asterix/samples/`
2. Add test case definition in `tools/pcap_validator.py`
3. Generate golden file: `python tools/pcap_validator.py --generate`
4. Commit both PCAP and golden files

Example test case:
```python
TestCase(
    name="my_new_test",
    pcap_file="my_test.pcap",
    format_flag="-P",
    output_format="-j",
    expected_categories=[48],
    golden_file="my_test_json.golden"
)
```

## Integration with CI

The PCAP validation runs automatically in CI via `.github/workflows/pcap-validation.yml`:

- Runs on all PRs touching parser code
- Compares against golden files
- Reports performance regressions (>10% slowdown)
- Uploads artifacts on failure for debugging

## Wireshark Plugin Validation

This framework also validates alignment between:
- Standalone ASTERIX parser
- Wireshark ASTERIX_EXT plugin (tshark)

```bash
# Compare parser vs tshark output (requires Wireshark)
python tests/pcap/tools/pcap_validator.py --wireshark
```

## Test Data Sources

- **install/share/asterix/samples/** - Standard test files from project
- **tests/pcap/data/** - Additional test cases
- **benchmarks/data/** - Performance test data

## Related Issues

- [#41](https://github.com/montge/asterix/issues/41) - PCAP validation framework
- [#22](https://github.com/montge/asterix/issues/22) - Wireshark 4.x plugin

## Troubleshooting

### "Executable not found"
Build and install the project first:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install
cmake --build build --parallel
cmake --install build
```

### "PCAP file not found"
Ensure sample data is installed:
```bash
cmake --install build  # Copies sample data to install/share/asterix/samples/
```

### "Golden file not found"
Generate golden files:
```bash
python tests/pcap/tools/pcap_validator.py --generate
```
