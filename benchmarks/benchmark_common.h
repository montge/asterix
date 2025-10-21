/*
 *  ASTERIX Performance Benchmark Framework
 *  Common utilities and timing functions
 *
 *  This file provides shared functionality for all benchmarks:
 *  - High-resolution timing
 *  - Statistical analysis (mean, median, percentiles)
 *  - System resource monitoring
 *  - JSON result output
 *  - Command-line argument parsing
 */

#ifndef BENCHMARK_COMMON_H
#define BENCHMARK_COMMON_H

#include <chrono>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <map>
#include <unistd.h>
#include <sys/resource.h>

#ifdef __linux__
#include <sys/sysinfo.h>
#endif

using namespace std::chrono;

// ============================================================================
// Configuration Structures
// ============================================================================

struct BenchmarkConfig {
    int iterations = 5;
    int warmup_iterations = 1;
    std::string output_file;
    std::string baseline_commit;
    bool verbose = false;
    bool ci_mode = false;
    double threshold = 0.95;  // For CI regression detection
};

struct SystemInfo {
    std::string cpu_model;
    int cpu_cores;
    long ram_mb;
    std::string os_version;
    std::string compiler;
    std::string build_flags;

    SystemInfo() {
        cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);

#ifdef __linux__
        // Read CPU model
        std::ifstream cpuinfo("/proc/cpuinfo");
        std::string line;
        while (std::getline(cpuinfo, line)) {
            if (line.find("model name") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    cpu_model = line.substr(pos + 2);
                    break;
                }
            }
        }

        // Read total RAM
        struct sysinfo si;
        if (sysinfo(&si) == 0) {
            ram_mb = si.totalram / (1024 * 1024);
        }

        // Read OS version
        std::ifstream os_release("/etc/os-release");
        while (std::getline(os_release, line)) {
            if (line.find("PRETTY_NAME") != std::string::npos) {
                size_t start = line.find("\"");
                size_t end = line.rfind("\"");
                if (start != std::string::npos && end != std::string::npos) {
                    os_version = line.substr(start + 1, end - start - 1);
                    break;
                }
            }
        }
#endif

        // Compiler info
#ifdef __GNUC__
        std::ostringstream compiler_ss;
        compiler_ss << "gcc " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__;
        compiler = compiler_ss.str();
#elif defined(__clang__)
        std::ostringstream compiler_ss;
        compiler_ss << "clang " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__;
        compiler = compiler_ss.str();
#else
        compiler = "unknown";
#endif

        // Build flags
#ifdef NDEBUG
        build_flags = "-O3 -DNDEBUG";
#else
        build_flags = "-O0 -g";
#endif
    }
};

// ============================================================================
// Timing and Statistics
// ============================================================================

class Timer {
private:
    high_resolution_clock::time_point start_time;
    high_resolution_clock::time_point end_time;
    bool running = false;

public:
    void start() {
        start_time = high_resolution_clock::now();
        running = true;
    }

    void stop() {
        end_time = high_resolution_clock::now();
        running = false;
    }

    double elapsed_ms() const {
        auto end = running ? high_resolution_clock::now() : end_time;
        return duration_cast<duration<double, std::milli>>(end - start_time).count();
    }

    double elapsed_us() const {
        auto end = running ? high_resolution_clock::now() : end_time;
        return duration_cast<duration<double, std::micro>>(end - start_time).count();
    }

    double elapsed_seconds() const {
        auto end = running ? high_resolution_clock::now() : end_time;
        return duration_cast<duration<double>>(end - start_time).count();
    }
};

class Statistics {
private:
    std::vector<double> values;

public:
    void add(double value) {
        values.push_back(value);
    }

    void clear() {
        values.clear();
    }

    size_t count() const {
        return values.size();
    }

    double mean() const {
        if (values.empty()) return 0.0;
        return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    }

    double median() const {
        if (values.empty()) return 0.0;
        std::vector<double> sorted = values;
        std::sort(sorted.begin(), sorted.end());
        size_t n = sorted.size();
        return n % 2 == 0 ? (sorted[n/2-1] + sorted[n/2]) / 2.0 : sorted[n/2];
    }

    double percentile(double p) const {
        if (values.empty()) return 0.0;
        std::vector<double> sorted = values;
        std::sort(sorted.begin(), sorted.end());
        size_t idx = static_cast<size_t>(p * (sorted.size() - 1));
        return sorted[idx];
    }

    double min() const {
        if (values.empty()) return 0.0;
        return *std::min_element(values.begin(), values.end());
    }

    double max() const {
        if (values.empty()) return 0.0;
        return *std::max_element(values.begin(), values.end());
    }

    double stddev() const {
        if (values.empty()) return 0.0;
        double m = mean();
        double sq_sum = std::accumulate(values.begin(), values.end(), 0.0,
            [m](double acc, double val) {
                return acc + (val - m) * (val - m);
            });
        return std::sqrt(sq_sum / values.size());
    }
};

// ============================================================================
// Resource Monitoring
// ============================================================================

struct ResourceUsage {
    long peak_rss_kb = 0;      // Peak resident set size in KB
    double cpu_time_sec = 0.0;  // CPU time used
    long voluntary_ctx_switches = 0;
    long involuntary_ctx_switches = 0;

    void capture() {
        struct rusage usage;
        if (getrusage(RUSAGE_SELF, &usage) == 0) {
            peak_rss_kb = usage.ru_maxrss;
#ifdef __linux__
            // On Linux, ru_maxrss is in KB
            // On macOS, it's in bytes
#else
            peak_rss_kb /= 1024;
#endif
            cpu_time_sec = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6 +
                          usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;
            voluntary_ctx_switches = usage.ru_nvcsw;
            involuntary_ctx_switches = usage.ru_nivcsw;
        }
    }

    double peak_rss_mb() const {
        return peak_rss_kb / 1024.0;
    }
};

// ============================================================================
// Benchmark Results
// ============================================================================

class BenchmarkResults {
private:
    std::string benchmark_name;
    std::string git_commit;
    SystemInfo system_info;
    ResourceUsage resource_usage;
    std::map<std::string, double> metrics;
    std::map<std::string, Statistics> stat_metrics;

public:
    BenchmarkResults(const std::string& name) : benchmark_name(name) {
        // Try to get git commit hash
        FILE* pipe = popen("git rev-parse --short HEAD 2>/dev/null", "r");
        if (pipe) {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                git_commit = buffer;
                // Remove trailing newline
                git_commit.erase(git_commit.find_last_not_of("\n\r") + 1);
            }
            pclose(pipe);
        }

        resource_usage.capture();
    }

    void add_metric(const std::string& name, double value) {
        metrics[name] = value;
    }

    void add_stat_value(const std::string& name, double value) {
        stat_metrics[name].add(value);
    }

    void finalize() {
        resource_usage.capture();
    }

    void print_summary() const {
        std::cout << "\n";
        std::cout << "============================================\n";
        std::cout << "  " << benchmark_name << " Results\n";
        std::cout << "============================================\n";
        std::cout << "Git Commit: " << git_commit << "\n";
        std::cout << "CPU: " << system_info.cpu_model << " (" << system_info.cpu_cores << " cores)\n";
        std::cout << "RAM: " << system_info.ram_mb << " MB\n";
        std::cout << "Compiler: " << system_info.compiler << "\n";
        std::cout << "--------------------------------------------\n";

        // Print metrics
        for (const auto& [name, value] : metrics) {
            std::cout << std::left << std::setw(30) << name << ": "
                     << std::fixed << std::setprecision(2) << value << "\n";
        }

        // Print statistical metrics
        for (const auto& [name, stats] : stat_metrics) {
            std::cout << std::left << std::setw(30) << (name + " (mean)") << ": "
                     << std::fixed << std::setprecision(2) << stats.mean() << "\n";
            std::cout << std::left << std::setw(30) << (name + " (median)") << ": "
                     << std::fixed << std::setprecision(2) << stats.median() << "\n";
            std::cout << std::left << std::setw(30) << (name + " (p95)") << ": "
                     << std::fixed << std::setprecision(2) << stats.percentile(0.95) << "\n";
            std::cout << std::left << std::setw(30) << (name + " (p99)") << ": "
                     << std::fixed << std::setprecision(2) << stats.percentile(0.99) << "\n";
        }

        std::cout << "--------------------------------------------\n";
        std::cout << std::left << std::setw(30) << "Peak Memory" << ": "
                 << std::fixed << std::setprecision(2) << resource_usage.peak_rss_mb() << " MB\n";
        std::cout << std::left << std::setw(30) << "CPU Time" << ": "
                 << std::fixed << std::setprecision(2) << resource_usage.cpu_time_sec << " sec\n";
        std::cout << "============================================\n";
        std::cout << std::endl;
    }

    bool save_json(const std::string& filename) const {
        if (filename.empty()) return false;

        std::ofstream out(filename);
        if (!out.is_open()) {
            std::cerr << "ERROR: Could not open output file: " << filename << "\n";
            return false;
        }

        out << "{\n";
        out << "  \"benchmark\": \"" << benchmark_name << "\",\n";
        out << "  \"timestamp\": \"" << get_timestamp() << "\",\n";
        out << "  \"git_commit\": \"" << git_commit << "\",\n";

        // Build config
        out << "  \"build_config\": {\n";
        out << "    \"compiler\": \"" << system_info.compiler << "\",\n";
        out << "    \"optimization\": \"" << system_info.build_flags << "\",\n";
        out << "    \"cxx_standard\": \"17\"\n";
        out << "  },\n";

        // System info
        out << "  \"system_info\": {\n";
        out << "    \"cpu\": \"" << system_info.cpu_model << "\",\n";
        out << "    \"cores\": " << system_info.cpu_cores << ",\n";
        out << "    \"ram_mb\": " << system_info.ram_mb << ",\n";
        out << "    \"os\": \"" << system_info.os_version << "\"\n";
        out << "  },\n";

        // Metrics
        out << "  \"metrics\": {\n";
        bool first = true;
        for (const auto& [name, value] : metrics) {
            if (!first) out << ",\n";
            out << "    \"" << name << "\": " << std::fixed << std::setprecision(4) << value;
            first = false;
        }

        // Statistical metrics
        for (const auto& [name, stats] : stat_metrics) {
            if (!first) out << ",\n";
            out << "    \"" << name << "_mean\": " << std::fixed << std::setprecision(4) << stats.mean() << ",\n";
            out << "    \"" << name << "_median\": " << std::fixed << std::setprecision(4) << stats.median() << ",\n";
            out << "    \"" << name << "_p95\": " << std::fixed << std::setprecision(4) << stats.percentile(0.95) << ",\n";
            out << "    \"" << name << "_p99\": " << std::fixed << std::setprecision(4) << stats.percentile(0.99) << ",\n";
            out << "    \"" << name << "_min\": " << std::fixed << std::setprecision(4) << stats.min() << ",\n";
            out << "    \"" << name << "_max\": " << std::fixed << std::setprecision(4) << stats.max() << ",\n";
            out << "    \"" << name << "_stddev\": " << std::fixed << std::setprecision(4) << stats.stddev();
            first = false;
        }
        out << "\n  },\n";

        // Resource usage
        out << "  \"resources\": {\n";
        out << "    \"peak_rss_mb\": " << std::fixed << std::setprecision(2) << resource_usage.peak_rss_mb() << ",\n";
        out << "    \"cpu_time_sec\": " << std::fixed << std::setprecision(4) << resource_usage.cpu_time_sec << ",\n";
        out << "    \"voluntary_ctx_switches\": " << resource_usage.voluntary_ctx_switches << ",\n";
        out << "    \"involuntary_ctx_switches\": " << resource_usage.involuntary_ctx_switches << "\n";
        out << "  }\n";
        out << "}\n";

        out.close();
        return true;
    }

private:
    std::string get_timestamp() const {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&now_c), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }
};

// ============================================================================
// Argument Parsing Helpers
// ============================================================================

inline BenchmarkConfig parse_common_args(int argc, char** argv) {
    BenchmarkConfig config;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--iterations" && i + 1 < argc) {
            config.iterations = std::atoi(argv[++i]);
        } else if (arg == "--warmup" && i + 1 < argc) {
            config.warmup_iterations = std::atoi(argv[++i]);
        } else if (arg == "--output" && i + 1 < argc) {
            config.output_file = argv[++i];
        } else if (arg == "--baseline" && i + 1 < argc) {
            config.baseline_commit = argv[++i];
        } else if (arg == "--verbose" || arg == "-v") {
            config.verbose = true;
        } else if (arg == "--ci") {
            config.ci_mode = true;
        } else if (arg == "--threshold" && i + 1 < argc) {
            config.threshold = std::atof(argv[++i]);
        }
    }

    return config;
}

inline void print_help(const std::string& program_name, const std::string& usage) {
    std::cout << "ASTERIX Performance Benchmark\n\n";
    std::cout << "Usage: " << program_name << " " << usage << "\n\n";
    std::cout << "Common Options:\n";
    std::cout << "  --iterations <n>    Number of benchmark iterations (default: 5)\n";
    std::cout << "  --warmup <n>        Number of warmup iterations (default: 1)\n";
    std::cout << "  --output <file>     Save results to JSON file\n";
    std::cout << "  --baseline <commit> Compare against baseline git commit\n";
    std::cout << "  --verbose, -v       Print verbose output\n";
    std::cout << "  --ci                CI mode (for regression testing)\n";
    std::cout << "  --threshold <val>   Performance threshold for CI (default: 0.95)\n";
    std::cout << "  --help, -h          Show this help message\n";
}

// ============================================================================
// Utility Functions
// ============================================================================

inline std::string format_bytes(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_idx = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unit_idx < 4) {
        size /= 1024.0;
        unit_idx++;
    }

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << size << " " << units[unit_idx];
    return ss.str();
}

inline std::string format_duration(double seconds) {
    if (seconds < 1e-3) {
        return std::to_string(static_cast<int>(seconds * 1e6)) + " μs";
    } else if (seconds < 1.0) {
        return std::to_string(static_cast<int>(seconds * 1e3)) + " ms";
    } else {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << seconds << " s";
        return ss.str();
    }
}

#endif // BENCHMARK_COMMON_H
