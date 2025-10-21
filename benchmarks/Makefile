#
# ASTERIX Performance Benchmark Framework
# Makefile (alternative to CMake)
#

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=native -Wall -Wextra -DNDEBUG
LDFLAGS = -pthread

# Directories
SRC_DIR = .
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin
OBJ_DIR = $(BUILD_DIR)/obj

# Include paths
INCLUDES = -I. -I../src/asterix -I../src/engine -I../src/main

# Targets
TARGETS = benchmark_pcap_processing benchmark_json_output benchmark_udp_multicast

# All binaries with full path
BINARIES = $(addprefix $(BIN_DIR)/, $(TARGETS))

# Default target
.PHONY: all
all: directories $(BINARIES)

# Create necessary directories
.PHONY: directories
directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p results
	@mkdir -p data

# PCAP Processing Benchmark
$(BIN_DIR)/benchmark_pcap_processing: benchmark_pcap_processing.cpp benchmark_common.h
	@echo "Building $@..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)

# JSON Output Benchmark
$(BIN_DIR)/benchmark_json_output: benchmark_json_output.cpp benchmark_common.h
	@echo "Building $@..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)

# UDP Multicast Benchmark
$(BIN_DIR)/benchmark_udp_multicast: benchmark_udp_multicast.cpp benchmark_common.h
	@echo "Building $@..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)

# Run all benchmarks
.PHONY: run
run: all
	@./run_benchmarks.sh

# Quick test (fewer iterations)
.PHONY: test
test: all
	@./run_benchmarks.sh --iterations 2 --warmup 1 --duration 5

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)

# Clean everything including results
.PHONY: distclean
distclean: clean
	@echo "Cleaning results and generated data..."
	@rm -rf results/* data/synthetic_*

# Install benchmarks (optional)
.PHONY: install
install: all
	@echo "Installing benchmarks to ../install/bin/..."
	@mkdir -p ../install/bin
	@cp $(BINARIES) ../install/bin/
	@echo "Installation complete"

# Help target
.PHONY: help
help:
	@echo "ASTERIX Benchmark Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all         - Build all benchmarks (default)"
	@echo "  run         - Build and run all benchmarks"
	@echo "  test        - Quick test run with fewer iterations"
	@echo "  clean       - Remove build artifacts"
	@echo "  distclean   - Remove build artifacts, results, and test data"
	@echo "  install     - Install benchmarks to ../install/bin/"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Benchmarks:"
	@echo "  $(TARGETS)"
	@echo ""
	@echo "Examples:"
	@echo "  make                  # Build all benchmarks"
	@echo "  make run              # Build and run all benchmarks"
	@echo "  make test             # Quick test run"
	@echo "  make clean            # Clean build"

# Print build configuration
.PHONY: config
config:
	@echo "Build Configuration:"
	@echo "  CXX:      $(CXX)"
	@echo "  CXXFLAGS: $(CXXFLAGS)"
	@echo "  LDFLAGS:  $(LDFLAGS)"
	@echo "  INCLUDES: $(INCLUDES)"
	@echo "  BIN_DIR:  $(BIN_DIR)"
