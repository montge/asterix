#!/bin/bash
#
# ASTERIX Performance Benchmark Runner
#
# This script orchestrates running all benchmarks, generating test data,
# comparing with baselines, and producing comprehensive reports.
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
RESULTS_DIR="${SCRIPT_DIR}/results"
DATA_DIR="${SCRIPT_DIR}/data"
ASTERIX_ROOT="$(dirname "${SCRIPT_DIR}")"

# Benchmark options
ITERATIONS=5
WARMUP=1
DURATION=10
VERBOSE=false
CI_MODE=false
THRESHOLD=0.95
BASELINE_COMMIT=""
REBUILD=false

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to print usage
usage() {
    cat << EOF
ASTERIX Performance Benchmark Runner

Usage: $0 [OPTIONS]

Options:
    -h, --help              Show this help message
    -i, --iterations N      Number of benchmark iterations (default: 5)
    -w, --warmup N          Number of warmup iterations (default: 1)
    -d, --duration N        Duration for UDP benchmark in seconds (default: 10)
    -v, --verbose           Enable verbose output
    --ci                    CI mode (regression testing)
    --threshold RATIO       Performance threshold for CI (default: 0.95)
    --baseline COMMIT       Compare against baseline git commit
    --rebuild               Force rebuild of benchmarks
    --clean                 Clean build and results directories

Examples:
    # Run all benchmarks with default settings
    $0

    # Run with more iterations for statistical confidence
    $0 --iterations 10 --warmup 2

    # Compare against baseline commit
    $0 --baseline a769202

    # CI mode with threshold
    $0 --ci --threshold 0.95

EOF
}

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -i|--iterations)
            ITERATIONS="$2"
            shift 2
            ;;
        -w|--warmup)
            WARMUP="$2"
            shift 2
            ;;
        -d|--duration)
            DURATION="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --ci)
            CI_MODE=true
            shift
            ;;
        --threshold)
            THRESHOLD="$2"
            shift 2
            ;;
        --baseline)
            BASELINE_COMMIT="$2"
            shift 2
            ;;
        --rebuild)
            REBUILD=true
            shift
            ;;
        --clean)
            print_info "Cleaning build and results directories..."
            rm -rf "${BUILD_DIR}" "${RESULTS_DIR}"
            print_success "Clean complete"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# Create directories
mkdir -p "${BUILD_DIR}"
mkdir -p "${RESULTS_DIR}"
mkdir -p "${DATA_DIR}"

# Get current timestamp for results
TIMESTAMP=$(date +%Y-%m-%d_%H-%M-%S)
CURRENT_RESULTS_DIR="${RESULTS_DIR}/${TIMESTAMP}"
mkdir -p "${CURRENT_RESULTS_DIR}"

# Symlink to latest
ln -sfn "${TIMESTAMP}" "${RESULTS_DIR}/latest"

# Build benchmarks
build_benchmarks() {
    print_info "Building benchmarks..."

    if [[ "${REBUILD}" == "true" ]] || [[ ! -f "${BUILD_DIR}/bin/benchmark_pcap_processing" ]]; then
        cd "${BUILD_DIR}"

        cmake -DCMAKE_BUILD_TYPE=Release \
              -DCMAKE_CXX_FLAGS="-O3 -march=native" \
              "${SCRIPT_DIR}"

        make -j$(nproc)

        cd "${SCRIPT_DIR}"
        print_success "Benchmarks built successfully"
    else
        print_info "Using existing build (use --rebuild to force rebuild)"
    fi
}

# Check for test data
check_test_data() {
    print_info "Checking for test data..."

    # Look for sample data in various locations
    SAMPLE_DATA_PATHS=(
        "${ASTERIX_ROOT}/asterix/sample_data"
        "${ASTERIX_ROOT}/install/share/asterix/samples"
        "${DATA_DIR}"
    )

    SAMPLE_PCAP=""
    for path in "${SAMPLE_DATA_PATHS[@]}"; do
        if [[ -f "${path}/cat_034_048.pcap" ]]; then
            SAMPLE_PCAP="${path}/cat_034_048.pcap"
            break
        fi
    done

    if [[ -z "${SAMPLE_PCAP}" ]]; then
        print_warning "No sample PCAP file found"
        print_info "Generating synthetic test data..."
        generate_test_data
    else
        print_success "Found sample data: ${SAMPLE_PCAP}"
    fi
}

# Generate synthetic test data
generate_test_data() {
    print_info "Generating synthetic ASTERIX data..."

    # Generate a simple ASTERIX data file
    python3 << 'EOF'
import struct
import sys

def create_asterix_record(category, sac, sic):
    """Create a minimal ASTERIX record"""
    # FSPEC (1 byte): Only I010 present
    fspec = bytes([0x80])

    # I010: Data Source Identifier (SAC/SIC)
    i010 = struct.pack('BB', sac, sic)

    # Calculate total length
    record = fspec + i010
    header = struct.pack('!BH', category, len(record) + 3)

    return header + record

# Generate test file with multiple records
output_file = 'data/synthetic_test.raw'
with open(output_file, 'wb') as f:
    # Generate 10000 records
    for i in range(10000):
        record = create_asterix_record(48, 1, i % 256)
        f.write(record)

print(f"Generated {output_file}")
EOF

    # Set the sample file
    SAMPLE_PCAP="${DATA_DIR}/synthetic_test.raw"
    print_success "Test data generated: ${SAMPLE_PCAP}"
}

# Run JSON output benchmark
run_json_benchmark() {
    print_info "Running JSON output benchmark..."

    local output_file="${CURRENT_RESULTS_DIR}/benchmark_json_output.json"
    local cmd="${BUILD_DIR}/bin/benchmark_json_output"

    cmd="${cmd} --records 10000"
    cmd="${cmd} --iterations ${ITERATIONS}"
    cmd="${cmd} --warmup ${WARMUP}"
    cmd="${cmd} --output ${output_file}"

    if [[ "${VERBOSE}" == "true" ]]; then
        cmd="${cmd} --verbose"
    fi

    if [[ "${CI_MODE}" == "true" ]]; then
        cmd="${cmd} --ci --threshold ${THRESHOLD}"
    fi

    eval "${cmd}"
    print_success "JSON benchmark complete"
}

# Run PCAP processing benchmark
run_pcap_benchmark() {
    if [[ -z "${SAMPLE_PCAP}" ]] || [[ ! -f "${SAMPLE_PCAP}" ]]; then
        print_warning "Skipping PCAP benchmark: no test file available"
        return
    fi

    print_info "Running PCAP processing benchmark..."

    local output_file="${CURRENT_RESULTS_DIR}/benchmark_pcap_processing.json"
    local cmd="${BUILD_DIR}/bin/benchmark_pcap_processing"

    cmd="${cmd} --iterations ${ITERATIONS}"
    cmd="${cmd} --warmup ${WARMUP}"
    cmd="${cmd} --output ${output_file}"
    cmd="${cmd} --format json"

    if [[ "${VERBOSE}" == "true" ]]; then
        cmd="${cmd} --verbose"
    fi

    if [[ "${CI_MODE}" == "true" ]]; then
        cmd="${cmd} --ci --threshold ${THRESHOLD}"
    fi

    cmd="${cmd} ${SAMPLE_PCAP}"

    eval "${cmd}"
    print_success "PCAP benchmark complete"
}

# Run UDP multicast benchmark
run_udp_benchmark() {
    print_info "Running UDP multicast benchmark..."

    local output_file="${CURRENT_RESULTS_DIR}/benchmark_udp_multicast.json"
    local cmd="${BUILD_DIR}/bin/benchmark_udp_multicast"

    cmd="${cmd} --duration ${DURATION}"
    cmd="${cmd} --output ${output_file}"
    cmd="${cmd} --rate 5000"  # 5000 packets/sec

    if [[ "${VERBOSE}" == "true" ]]; then
        cmd="${cmd} --verbose"
    fi

    if [[ "${CI_MODE}" == "true" ]]; then
        cmd="${cmd} --ci --threshold ${THRESHOLD}"
    fi

    eval "${cmd}"
    print_success "UDP benchmark complete"
}

# Generate summary report
generate_summary() {
    print_info "Generating summary report..."

    cat > "${CURRENT_RESULTS_DIR}/summary.txt" << EOF
ASTERIX Performance Benchmark Summary
======================================
Timestamp: ${TIMESTAMP}
Git Commit: $(git -C "${ASTERIX_ROOT}" rev-parse --short HEAD 2>/dev/null || echo "unknown")
Iterations: ${ITERATIONS}
Warmup: ${WARMUP}

EOF

    # Extract key metrics from JSON files
    for json_file in "${CURRENT_RESULTS_DIR}"/*.json; do
        if [[ -f "${json_file}" ]]; then
            echo "=== $(basename ${json_file}) ===" >> "${CURRENT_RESULTS_DIR}/summary.txt"

            # Use Python to extract metrics if available, otherwise basic grep
            if command -v python3 &> /dev/null; then
                python3 << EOF >> "${CURRENT_RESULTS_DIR}/summary.txt"
import json
import sys

try:
    with open('${json_file}', 'r') as f:
        data = json.load(f)

    metrics = data.get('metrics', {})
    for key, value in sorted(metrics.items()):
        if isinstance(value, float):
            print(f"  {key}: {value:.2f}")
        else:
            print(f"  {key}: {value}")
except Exception as e:
    print(f"  Error parsing JSON: {e}", file=sys.stderr)
EOF
            fi
            echo "" >> "${CURRENT_RESULTS_DIR}/summary.txt"
        fi
    done

    print_success "Summary report generated: ${CURRENT_RESULTS_DIR}/summary.txt"
}

# Compare with baseline
compare_with_baseline() {
    if [[ -z "${BASELINE_COMMIT}" ]]; then
        return
    fi

    print_info "Comparing with baseline commit: ${BASELINE_COMMIT}"

    # TODO: Implement baseline comparison
    # This would:
    # 1. Stash current changes
    # 2. Checkout baseline commit
    # 3. Build and run benchmarks
    # 4. Return to current commit
    # 5. Generate comparison report

    print_warning "Baseline comparison not yet implemented"
}

# Main execution
main() {
    echo ""
    echo "======================================"
    echo "  ASTERIX Performance Benchmarks"
    echo "======================================"
    echo ""

    print_info "Configuration:"
    echo "  Iterations: ${ITERATIONS}"
    echo "  Warmup: ${WARMUP}"
    echo "  Duration: ${DURATION}s"
    echo "  CI Mode: ${CI_MODE}"
    echo "  Results: ${CURRENT_RESULTS_DIR}"
    echo ""

    # Build
    build_benchmarks

    # Check test data
    check_test_data

    # Run benchmarks
    echo ""
    echo "======================================"
    echo "  Running Benchmarks"
    echo "======================================"
    echo ""

    run_json_benchmark
    echo ""

    run_pcap_benchmark
    echo ""

    run_udp_benchmark
    echo ""

    # Generate reports
    generate_summary

    # Compare with baseline if requested
    compare_with_baseline

    # Final summary
    echo ""
    echo "======================================"
    echo "  Benchmark Results"
    echo "======================================"
    cat "${CURRENT_RESULTS_DIR}/summary.txt"

    print_success "All benchmarks complete!"
    print_info "Results saved to: ${CURRENT_RESULTS_DIR}"
    print_info "View summary: cat ${RESULTS_DIR}/latest/summary.txt"

    # Exit code for CI
    if [[ "${CI_MODE}" == "true" ]]; then
        # Check if any benchmark failed
        # (Individual benchmarks will have exited with error if they failed)
        exit 0
    fi
}

# Run main
main
