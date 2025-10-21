# Benchmark Test Data

This directory contains test data files for benchmarking ASTERIX decoder performance.

## Test Data Sources

### Automatic Generation

The `run_benchmarks.sh` script will automatically generate synthetic test data if no sample files are found.

### Using Real Sample Data

For more realistic benchmarks, use actual ASTERIX data files:

1. Copy sample files to this directory:
   ```bash
   cp ../asterix/sample_data/*.pcap ./
   cp ../asterix/sample_data/*.raw ./
   ```

2. Or symlink to existing sample data:
   ```bash
   ln -s ../asterix/sample_data/* ./
   ```

## Generating Custom Test Data

### Small Test Set (10 MB)

```bash
./generate_test_data.sh --size small
```

### Medium Test Set (100 MB)

```bash
./generate_test_data.sh --size medium
```

### Large Test Set (1 GB)

```bash
./generate_test_data.sh --size large
```

## Test Data Characteristics

### Synthetic Data

Generated data includes:
- Valid ASTERIX Category 048 records
- Incrementing sequence numbers
- Minimal but valid data items
- Configurable size and record count

### Real Sample Data

Available sample files:
- `cat_034_048.pcap` - Mixed CAT034/CAT048 in PCAP format
- `cat048.raw` - Raw CAT048 data
- `cat062cat065.raw` - Mixed CAT062/CAT065 data

## File Formats

### Raw ASTERIX (.raw)

Binary ASTERIX data blocks without any encapsulation:
```
[CAT(1)][LEN(2)][DATA...] [CAT(1)][LEN(2)][DATA...] ...
```

### PCAP (.pcap)

PCAP file format with UDP packets containing ASTERIX data:
```
[PCAP Global Header][Packet Header][Ethernet][IP][UDP][ASTERIX]...
```

## Data Size Guidelines

For different benchmark scenarios:

| Scenario | Recommended Size | File Type | Notes |
|----------|------------------|-----------|-------|
| Quick Test | 1-10 MB | .raw | Fast iteration during development |
| Standard Benchmark | 100 MB | .pcap | Realistic performance measurement |
| Stress Test | 1 GB+ | .pcap | Memory and sustained throughput testing |
| Latency Test | Small packets | .pcap | Focus on per-packet overhead |

## Cleaning Up

Remove all generated test data:

```bash
./clean_test_data.sh
```

Or manually:

```bash
rm -f *.raw *.pcap
```

## .gitignore

Large test data files are gitignored. Only small sample files should be committed to the repository.
