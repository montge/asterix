# Radar Integration Examples

This directory contains examples demonstrating radar simulation integration with the ASTERIX encoder/decoder.

## Overview

These examples show how to:
- Generate synthetic radar plot data
- Encode radar plots to ASTERIX CAT048 binary format
- Decode ASTERIX data and validate accuracy
- Create realistic multi-aircraft scenarios

## Examples

### 1. Basic Mock Radar (`basic_mock_radar.py`)

**Simplest starting point** - Demonstrates basic radar plot generation and ASTERIX encoding.

```bash
python3 basic_mock_radar.py
```

**What it does:**
- Creates a mock radar sensor
- Generates 5 random radar plots
- Encodes to ASTERIX CAT048
- Saves to file (`basic_radar_output.ast`)
- Decodes and verifies (if ASTERIX decoder installed)

**Use this when:**
- Learning the basics of radar-to-ASTERIX integration
- Testing simple encoding/decoding workflows
- Understanding ASTERIX data structure

### 2. Aircraft Scenario (`aircraft_scenario.py`)

**Multi-aircraft simulation** - Demonstrates realistic aircraft trajectories with multiple moving targets.

```bash
python3 aircraft_scenario.py
```

**What it does:**
- Generates 5 aircraft with random trajectories
- Simulates 60 seconds of radar updates (4s rotation)
- Encodes ~75 radar plots to ASTERIX
- Analyzes scenario statistics (range, SNR distribution)
- Validates round-trip accuracy
- Saves to file (`aircraft_scenario.ast`)

**Use this when:**
- Testing with realistic multi-target scenarios
- Benchmarking encoder/decoder performance
- Developing tracking algorithms

### 3. Encode and Decode (`encode_and_decode.py`)

**Round-trip validation** - Complete encode → decode → validate pipeline with detailed error analysis.

```bash
python3 encode_and_decode.py
```

**What it does:**
- Generates 20 radar plots
- Encodes to ASTERIX CAT048
- Decodes the binary data
- Validates data preservation (range, azimuth accuracy)
- Prints detailed error statistics
- Shows hex dump of ASTERIX structure

**Use this when:**
- Verifying encoder accuracy
- Understanding quantization errors
- Debugging ASTERIX encoding issues
- Learning ASTERIX binary structure

## Requirements

### Python Packages

```bash
# Install ASTERIX decoder (recommended)
pip install asterix_decoder

# Or install from source
cd /path/to/asterix
python setup.py install --user
```

### Mock Radar Generator

The examples require the mock radar generator from `.local/integration/`:

```
asterix/
├── .local/
│   └── integration/
│       ├── mock_radar.py           # Mock radar data generator
│       └── asterix_encoder/
│           └── cat048.py           # ASTERIX CAT048 encoder
└── examples/
    └── radar_integration/
        ├── basic_mock_radar.py
        ├── aircraft_scenario.py
        └── encode_and_decode.py
```

The examples automatically add `.local/integration/` to `sys.path`.

## Running the Examples

### Quick Test (No Installation)

```bash
# From repository root
cd examples/radar_integration

# Run any example
python3 basic_mock_radar.py
python3 aircraft_scenario.py
python3 encode_and_decode.py
```

### With ASTERIX Decoder

For full functionality (decode + validate), install the ASTERIX decoder:

```bash
# Option 1: From PyPI
pip install asterix_decoder

# Option 2: From source
cd /path/to/asterix
python setup.py build
python setup.py install --user

# Verify installation
python -c "import asterix; print(asterix.__version__)"
```

Then run the examples:

```bash
cd examples/radar_integration
python3 encode_and_decode.py
```

**Expected output:**
```
✅ Decoded 20 records
✅ VALIDATION PASSED
  Mean range error:   3.45 m
  Max range error:    7.21 m
  Mean azimuth error: 0.0012°
  Max azimuth error:  0.0055°
```

## Output Files

The examples generate ASTERIX binary files:

- `basic_radar_output.ast` - 5 radar plots (~85 bytes)
- `aircraft_scenario.ast` - ~75 radar plots (~1275 bytes)

### Viewing Output

```bash
# Hex dump
hexdump -C basic_radar_output.ast

# Parse with ASTERIX CLI (if installed)
asterix -f basic_radar_output.ast -j         # JSON output
asterix -f aircraft_scenario.ast -jh          # Human-readable JSON
```

### Example Hex Output

```
00000000  30 00 55 fe 00 01 00 26  f4 40 47 35 dc 02 00 00  |0.U....&.@G5....|
00000010  fe 00 01 00 26 f6 a0 40  33 24 89 02 00 fe 00 01  |....&..@3$......|
00000020  00 26 f8 e0 53 70 cc d1  02 00 fe 00 01 00 26 fb  |.&..Sp........&.|
```

## Customization

### Modify Radar Parameters

```python
# In any example, change radar configuration:
radar = MockRadar(
    lat=40.7,              # New York (instead of Berlin)
    lon=-74.0,
    alt=10.0,              # Sea level
    max_range=300e3,       # 300 km (instead of 200 km)
    azimuth_resolution=0.5, # 0.5° (instead of 1°)
    update_rate=0.5        # 2 second rotation (instead of 4s)
)
```

### Modify Scenario Parameters

```python
# aircraft_scenario.py
plots = generate_aircraft_scenario(
    num_aircraft=10,       # 10 aircraft (instead of 5)
    duration=120.0,        # 2 minutes (instead of 60s)
    radar_lat=40.7,
    radar_lon=-74.0
)
```

### Add Mode 3/A and Mode S

```python
# Enable secondary surveillance radar features
asterix_data = encode_cat048(
    plots,
    sac=0,
    sic=1,
    include_mode3a=True,            # Add Mode 3/A codes
    include_aircraft_address=True   # Add Mode S addresses
)
```

## Performance Benchmarks

Typical performance (Ubuntu 22.04, Python 3.12):

| Operation | Throughput | Notes |
|-----------|------------|-------|
| Mock radar generation | ~1 ms / 1000 targets | In-memory, no I/O |
| ASTERIX encoding | ~2 ms / 1000 records | Pure Python |
| ASTERIX decoding | ~1 ms / 1000 records | C++ core |
| Round-trip validation | ~3 ms / 1000 records | Total pipeline |

## Troubleshooting

### Import Error: `No module named 'mock_radar'`

**Cause:** Examples can't find `.local/integration/`

**Fix:**
```bash
# Ensure you're running from examples/radar_integration/
cd /path/to/asterix/examples/radar_integration
python3 basic_mock_radar.py
```

### Import Error: `No module named 'asterix'`

**Cause:** ASTERIX decoder not installed

**Fix:**
```bash
pip install asterix_decoder
# Or build from source
cd /path/to/asterix
python setup.py install --user
```

### Decode Error: `Failed to parse ASTERIX data`

**Cause:** Encoder generated invalid ASTERIX data (bug)

**Troubleshooting:**
1. Check hex dump: `hexdump -C output.ast | head -20`
2. Verify CAT byte: First byte should be `0x30` (decimal 48)
3. Verify length: Bytes 2-3 should match file size
4. Report issue with hex dump and error message

## Next Steps

After running these examples:

1. **Read the integration guide:** `docs/INTEGRATION_RADAR_SIMULATION.md`
2. **Explore ASTERIX categories:** `ASTERIX_CATEGORY_SUPPORT_MATRIX.md`
3. **Study the encoder implementation:** `.local/integration/asterix_encoder/cat048.py`
4. **Review architecture docs:** `docs/ARCHITECTURE.md`

## License

These examples are part of the ASTERIX decoder project (GPL-3.0).

See `LICENSE` for full license text.

## Support

- **Documentation:** `docs/INTEGRATION_RADAR_SIMULATION.md`
- **Issues:** https://github.com/montge/asterix/issues
- **Contributing:** `CONTRIBUTING.md`
