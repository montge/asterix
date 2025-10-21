#!/bin/bash
#
# ASTERIX Test Data Generator
#
# Generates synthetic ASTERIX data files for benchmarking
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Default configuration
SIZE="medium"
OUTPUT_DIR="${SCRIPT_DIR}"

# Size definitions (in number of records)
declare -A SIZE_RECORDS=(
    ["small"]=10000
    ["medium"]=100000
    ["large"]=1000000
)

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --size)
            SIZE="$2"
            shift 2
            ;;
        --output)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        --help|-h)
            cat << EOF
ASTERIX Test Data Generator

Usage: $0 [OPTIONS]

Options:
    --size SIZE         Size of test data: small|medium|large (default: medium)
    --output DIR        Output directory (default: current directory)
    --help, -h          Show this help message

Sizes:
    small       ~1 MB    (10,000 records)
    medium      ~10 MB   (100,000 records)
    large       ~100 MB  (1,000,000 records)

EOF
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Validate size
if [[ ! -v SIZE_RECORDS[$SIZE] ]]; then
    echo "ERROR: Invalid size '$SIZE'. Must be one of: small, medium, large"
    exit 1
fi

NUM_RECORDS=${SIZE_RECORDS[$SIZE]}
OUTPUT_FILE="${OUTPUT_DIR}/synthetic_${SIZE}.raw"

echo "Generating ASTERIX test data..."
echo "  Size: ${SIZE} (${NUM_RECORDS} records)"
echo "  Output: ${OUTPUT_FILE}"

# Generate using Python
python3 << EOF
import struct
import random
import sys

def create_asterix_cat048_record(record_num):
    """Create a realistic CAT048 (Radar Data) record"""

    # Build data items
    items = []

    # I010: Data Source Identifier (SAC/SIC) - always present
    sac = random.randint(1, 255)
    sic = random.randint(1, 255)
    items.append(('010', struct.pack('BB', sac, sic)))

    # I140: Time of Day (3 bytes, 1/128 sec resolution)
    tod = random.randint(0, 86400 * 128)  # One day in 1/128 seconds
    items.append(('140', struct.pack('!I', tod)[1:]))  # Take last 3 bytes

    # I020: Target Report Descriptor (variable length, we'll use 1 byte)
    trd = random.randint(0, 255)
    items.append(('020', struct.pack('B', trd)))

    # I040: Measured Position in Polar Coordinates (4 bytes)
    rho = random.randint(0, 65535)  # Range in 1/256 NM
    theta = random.randint(0, 65535)  # Azimuth in 360/2^16 degrees
    items.append(('040', struct.pack('!HH', rho, theta)))

    # I070: Mode-3/A Code (2 bytes)
    mode3a = random.randint(0, 0o7777)
    items.append(('070', struct.pack('!H', mode3a)))

    # I090: Flight Level (2 bytes)
    fl = random.randint(0, 65535)  # Flight level in 1/4 FL
    items.append(('090', struct.pack('!H', fl)))

    # I220: Aircraft Address (3 bytes)
    addr = random.randint(0, 0xFFFFFF)
    items.append(('220', struct.pack('!I', addr)[1:]))  # Take last 3 bytes

    # Build FSPEC (Field Specification)
    # For simplicity, we'll use a fixed FSPEC indicating which items are present
    # FSPEC bits: I010=1, I140=1, I020=1, I040=1, I070=1, I090=1, I220=1
    # Binary: 1111111 0 (bit 8 is FX=0, no extension)
    fspec = bytes([0xFE])

    # Concatenate all items
    data = fspec
    for item_id, item_data in items:
        data += item_data

    # Add ASTERIX header: CAT(1) + LEN(2)
    total_len = len(data) + 3
    header = struct.pack('!BH', 48, total_len)

    return header + data

# Generate records
output_file = '${OUTPUT_FILE}'
num_records = ${NUM_RECORDS}

print(f"Writing {num_records} records to {output_file}...", file=sys.stderr)

with open(output_file, 'wb') as f:
    for i in range(num_records):
        record = create_asterix_cat048_record(i)
        f.write(record)

        if (i + 1) % 10000 == 0:
            print(f"  Progress: {i + 1}/{num_records} records", file=sys.stderr)

# Print file size
import os
file_size = os.path.getsize(output_file)
size_mb = file_size / (1024 * 1024)
print(f"Generated {output_file}", file=sys.stderr)
print(f"  Records: {num_records}", file=sys.stderr)
print(f"  File size: {file_size} bytes ({size_mb:.2f} MB)", file=sys.stderr)
print(f"  Avg record size: {file_size / num_records:.1f} bytes", file=sys.stderr)

EOF

echo "Done!"
