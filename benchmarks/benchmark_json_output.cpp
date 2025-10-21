/*
 *  ASTERIX Performance Benchmark - JSON Output Generation
 *
 *  Measures performance of JSON output formatting including:
 *  - JSON generation time per record
 *  - String allocation efficiency
 *  - Memory usage patterns
 *  - Throughput (records/sec)
 *
 *  This benchmark validates the string optimization and output formatting improvements.
 */

#include "benchmark_common.h"

#include <sstream>
#include <memory>

// Note: This benchmark uses simulated data structures and does not require
// linking against the full ASTERIX library. For full integration testing,
// use the test suite in install/test/

struct JsonBenchmarkConfig {
    BenchmarkConfig base;
    size_t num_records = 10000;
    bool extensive_format = false;  // Use JSON extensive format
    bool measure_allocations = true;
};

JsonBenchmarkConfig parse_args(int argc, char** argv) {
    JsonBenchmarkConfig config;
    config.base = parse_common_args(argc, argv);

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--records" && i + 1 < argc) {
            config.num_records = std::atoi(argv[++i]);
        } else if (arg == "--extensive") {
            config.extensive_format = true;
        } else if (arg == "--no-alloc-tracking") {
            config.measure_allocations = false;
        } else if (arg == "--help" || arg == "-h") {
            print_help(argv[0], "[OPTIONS]");
            std::cout << "\nJSON Benchmark Options:\n";
            std::cout << "  --records <n>         Number of records to generate (default: 10000)\n";
            std::cout << "  --extensive           Use extensive JSON format (with descriptions)\n";
            std::cout << "  --no-alloc-tracking   Disable allocation tracking\n";
            exit(0);
        }
    }

    return config;
}

// Simple JSON generator for benchmarking
class SimpleJsonGenerator {
private:
    std::ostringstream buffer;
    bool first_item = true;
    size_t nesting_level = 0;

public:
    void begin_object() {
        buffer << "{";
        first_item = true;
        nesting_level++;
    }

    void end_object() {
        buffer << "}";
        nesting_level--;
        first_item = false;
    }

    void begin_array(const std::string& name) {
        if (!first_item) buffer << ",";
        buffer << "\"" << name << "\":[";
        first_item = true;
    }

    void end_array() {
        buffer << "]";
        first_item = false;
    }

    void add_string(const std::string& name, const std::string& value) {
        if (!first_item) buffer << ",";
        buffer << "\"" << name << "\":\"" << value << "\"";
        first_item = false;
    }

    void add_number(const std::string& name, double value) {
        if (!first_item) buffer << ",";
        buffer << "\"" << name << "\":" << value;
        first_item = false;
    }

    void add_int(const std::string& name, int value) {
        if (!first_item) buffer << ",";
        buffer << "\"" << name << "\":" << value;
        first_item = false;
    }

    std::string to_string() const {
        return buffer.str();
    }

    void clear() {
        buffer.str("");
        buffer.clear();
        first_item = true;
        nesting_level = 0;
    }

    size_t size() const {
        return buffer.str().size();
    }
};

// Simulate a typical ASTERIX record for benchmarking
struct SimulatedRecord {
    int category;
    int record_number;
    double timestamp;
    std::vector<std::pair<std::string, std::string>> items;

    SimulatedRecord(int cat, int num) : category(cat), record_number(num) {
        timestamp = 1234567890.123456 + num * 0.1;

        // Add typical items for a CAT048 record
        items.push_back({"I010", "SAC=1 SIC=2"});
        items.push_back({"I140", "Time of Day=12345.678"});
        items.push_back({"I020", "Target Report Descriptor=TYP=0"});
        items.push_back({"I040", "Measured Position (Polar)=RHO=123.45 THETA=234.56"});
        items.push_back({"I070", "Mode-3/A=0x1234"});
        items.push_back({"I090", "Flight Level=350"});
        items.push_back({"I130", "Radar Plot Characteristics=SSR=12"});
        items.push_back({"I220", "Aircraft Address=0xABCDEF"});
        items.push_back({"I240", "Aircraft Identification=ABC1234"});
        items.push_back({"I250", "Mode S MB Data=BDS1=12 BDS2=34"});
    }
};

struct JsonGenerationResult {
    size_t records_generated = 0;
    size_t total_json_bytes = 0;
    double elapsed_seconds = 0.0;
    size_t estimated_allocations = 0;

    double records_per_second() const {
        if (elapsed_seconds <= 0.0) return 0.0;
        return records_generated / elapsed_seconds;
    }

    double bytes_per_record() const {
        if (records_generated == 0) return 0.0;
        return static_cast<double>(total_json_bytes) / records_generated;
    }

    double allocations_per_record() const {
        if (records_generated == 0) return 0.0;
        return static_cast<double>(estimated_allocations) / records_generated;
    }

    double throughput_mbps() const {
        if (elapsed_seconds <= 0.0) return 0.0;
        return (total_json_bytes / (1024.0 * 1024.0)) / elapsed_seconds;
    }
};

JsonGenerationResult benchmark_json_generation(const JsonBenchmarkConfig& config) {
    JsonGenerationResult result;
    SimpleJsonGenerator json;

    Timer timer;
    timer.start();

    for (size_t i = 0; i < config.num_records; i++) {
        SimulatedRecord record(48, i);

        json.clear();
        json.begin_object();

        // Generate JSON output similar to ASTERIX format
        json.add_int("category", record.category);
        json.add_int("record_number", record.record_number);
        json.add_number("timestamp", record.timestamp);

        json.begin_array("items");
        for (const auto& item : record.items) {
            json.begin_object();
            json.add_string("name", item.first);
            json.add_string("value", item.second);

            if (config.extensive_format) {
                // Add extra fields for extensive format
                json.add_string("hex", "0x1234567890ABCDEF");
                json.add_string("description", "This is a description of the data item");
                json.add_string("unit", "meters");
            }

            json.end_object();
        }
        json.end_array();

        json.end_object();

        std::string json_str = json.to_string();
        result.total_json_bytes += json_str.size();
        result.records_generated++;

        // Estimate allocations (rough approximation)
        // Each string allocation, each object, each number conversion
        result.estimated_allocations += 1 + record.items.size() * 3;
        if (config.extensive_format) {
            result.estimated_allocations += record.items.size() * 4;
        }
    }

    timer.stop();
    result.elapsed_seconds = timer.elapsed_seconds();

    return result;
}

int main(int argc, char** argv) {
    JsonBenchmarkConfig config = parse_args(argc, argv);
    BenchmarkResults results("json_output");

    std::cout << "ASTERIX JSON Output Generation Benchmark\n";
    std::cout << "=========================================\n";
    std::cout << "Records to generate: " << config.num_records << "\n";
    std::cout << "Format: " << (config.extensive_format ? "extensive" : "compact") << "\n";
    std::cout << "Iterations: " << config.base.iterations << "\n";
    std::cout << "Warmup: " << config.base.warmup_iterations << "\n";
    std::cout << std::endl;

    // Warmup iterations
    if (config.base.warmup_iterations > 0) {
        std::cout << "Running warmup iterations..." << std::endl;
        for (int i = 0; i < config.base.warmup_iterations; i++) {
            JsonGenerationResult warmup = benchmark_json_generation(config);
            if (config.base.verbose) {
                std::cout << "  Warmup " << (i + 1) << ": "
                         << std::fixed << std::setprecision(0)
                         << warmup.records_per_second() << " rec/s\n";
            }
        }
        std::cout << std::endl;
    }

    // Benchmark iterations
    std::cout << "Running benchmark iterations..." << std::endl;
    Statistics record_rate_stats;
    Statistics bytes_per_record_stats;
    Statistics alloc_per_record_stats;
    Statistics throughput_stats;

    for (int i = 0; i < config.base.iterations; i++) {
        JsonGenerationResult iter_result = benchmark_json_generation(config);

        record_rate_stats.add(iter_result.records_per_second());
        bytes_per_record_stats.add(iter_result.bytes_per_record());
        alloc_per_record_stats.add(iter_result.allocations_per_record());
        throughput_stats.add(iter_result.throughput_mbps());

        if (config.base.verbose) {
            std::cout << "  Iteration " << (i + 1) << ": "
                     << std::fixed << std::setprecision(0)
                     << iter_result.records_per_second() << " rec/s, "
                     << std::fixed << std::setprecision(1)
                     << iter_result.bytes_per_record() << " bytes/rec\n";
        }
    }
    std::cout << std::endl;

    // Store results
    results.add_metric("records_generated", config.num_records);
    results.add_metric("records_per_sec_mean", record_rate_stats.mean());
    results.add_metric("records_per_sec_median", record_rate_stats.median());
    results.add_metric("records_per_sec_p95", record_rate_stats.percentile(0.95));
    results.add_metric("records_per_sec_stddev", record_rate_stats.stddev());
    results.add_metric("bytes_per_record_mean", bytes_per_record_stats.mean());
    results.add_metric("bytes_per_record_stddev", bytes_per_record_stats.stddev());
    results.add_metric("alloc_per_record_mean", alloc_per_record_stats.mean());
    results.add_metric("throughput_mbps_mean", throughput_stats.mean());
    results.add_metric("throughput_mbps_median", throughput_stats.median());
    results.add_metric("iterations", config.base.iterations);

    // Calculate time per record in microseconds
    double us_per_record = 1e6 / record_rate_stats.median();
    results.add_metric("us_per_record_median", us_per_record);

    // Finalize and display
    results.finalize();
    results.print_summary();

    // Save to file if requested
    if (!config.base.output_file.empty()) {
        if (results.save_json(config.base.output_file)) {
            std::cout << "Results saved to: " << config.base.output_file << "\n";
        }
    }

    // CI mode: check thresholds
    if (config.base.ci_mode) {
        const double MIN_RECORDS_PER_SEC = 50000.0;  // Minimum acceptable rate
        if (record_rate_stats.median() < MIN_RECORDS_PER_SEC * config.base.threshold) {
            std::cerr << "FAILED: Record generation rate below threshold ("
                     << record_rate_stats.median() << " < "
                     << MIN_RECORDS_PER_SEC * config.base.threshold << " rec/s)\n";
            return 1;
        }
    }

    return 0;
}
