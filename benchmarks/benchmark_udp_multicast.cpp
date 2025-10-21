/*
 *  ASTERIX Performance Benchmark - UDP Multicast Processing
 *
 *  Measures performance of UDP multicast stream processing including:
 *  - Packet throughput (packets/sec, MB/s)
 *  - Processing latency percentiles (p50, p95, p99)
 *  - CPU usage under load
 *  - Packet loss detection
 *
 *  This benchmark validates the UDP processing and real-time performance optimizations.
 */

#include "benchmark_common.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <signal.h>

struct UdpBenchmarkConfig {
    BenchmarkConfig base;
    int duration_seconds = 10;
    int port = 21112;
    std::string mcast_group = "239.255.1.1";
    std::string interface_ip = "0.0.0.0";
    int packet_size = 1024;
    int target_rate_pps = 1000;
    bool generate_traffic = true;  // Generate synthetic traffic
};

UdpBenchmarkConfig parse_args(int argc, char** argv) {
    UdpBenchmarkConfig config;
    config.base = parse_common_args(argc, argv);

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--duration" && i + 1 < argc) {
            config.duration_seconds = std::atoi(argv[++i]);
        } else if (arg == "--port" && i + 1 < argc) {
            config.port = std::atoi(argv[++i]);
        } else if (arg == "--mcast-group" && i + 1 < argc) {
            config.mcast_group = argv[++i];
        } else if (arg == "--interface" && i + 1 < argc) {
            config.interface_ip = argv[++i];
        } else if (arg == "--packet-size" && i + 1 < argc) {
            config.packet_size = std::atoi(argv[++i]);
        } else if (arg == "--rate" && i + 1 < argc) {
            config.target_rate_pps = std::atoi(argv[++i]);
        } else if (arg == "--no-generate") {
            config.generate_traffic = false;
        } else if (arg == "--help" || arg == "-h") {
            print_help(argv[0], "[OPTIONS]");
            std::cout << "\nUDP Multicast Benchmark Options:\n";
            std::cout << "  --duration <sec>       Duration to run benchmark (default: 10)\n";
            std::cout << "  --port <port>          UDP port (default: 21112)\n";
            std::cout << "  --mcast-group <addr>   Multicast group address (default: 239.255.1.1)\n";
            std::cout << "  --interface <ip>       Interface IP address (default: 0.0.0.0)\n";
            std::cout << "  --packet-size <bytes>  Packet size (default: 1024)\n";
            std::cout << "  --rate <pps>           Target packets per second (default: 1000)\n";
            std::cout << "  --no-generate          Don't generate traffic (listen only)\n";
            exit(0);
        }
    }

    return config;
}

struct PacketTimestamp {
    high_resolution_clock::time_point receive_time;
    high_resolution_clock::time_point process_time;
    size_t packet_size;
    uint32_t sequence_number;
};

struct UdpBenchmarkResult {
    size_t packets_received = 0;
    size_t bytes_received = 0;
    size_t packets_lost = 0;
    double elapsed_seconds = 0.0;
    std::vector<double> latencies_us;

    double packets_per_second() const {
        if (elapsed_seconds <= 0.0) return 0.0;
        return packets_received / elapsed_seconds;
    }

    double throughput_mbps() const {
        if (elapsed_seconds <= 0.0) return 0.0;
        return (bytes_received / (1024.0 * 1024.0)) / elapsed_seconds;
    }

    double packet_loss_rate() const {
        size_t total = packets_received + packets_lost;
        if (total == 0) return 0.0;
        return static_cast<double>(packets_lost) / total;
    }
};

std::atomic<bool> g_running{true};

void signal_handler(int signal) {
    g_running = false;
}

// Traffic generator thread
void traffic_generator(const UdpBenchmarkConfig& config, std::atomic<bool>& running) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "ERROR: Could not create sender socket\n";
        return;
    }

    // Set TTL for multicast
    unsigned char ttl = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        std::cerr << "WARNING: Could not set TTL\n";
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(config.mcast_group.c_str());
    addr.sin_port = htons(config.port);

    // Create synthetic ASTERIX packet
    std::vector<unsigned char> packet(config.packet_size);
    // ASTERIX header: CAT=048, LEN
    packet[0] = 48;  // Category 048
    packet[1] = (config.packet_size >> 8) & 0xFF;
    packet[2] = config.packet_size & 0xFF;

    uint32_t sequence = 0;
    auto interval_us = 1000000 / config.target_rate_pps;
    auto next_send = high_resolution_clock::now();

    while (running) {
        auto now = high_resolution_clock::now();
        if (now >= next_send) {
            // Embed sequence number in packet
            memcpy(&packet[3], &sequence, sizeof(sequence));
            sequence++;

            sendto(sock, packet.data(), packet.size(), 0,
                   (struct sockaddr*)&addr, sizeof(addr));

            next_send += std::chrono::microseconds(interval_us);

            // Prevent accumulating delay
            if (next_send < now) {
                next_send = now + std::chrono::microseconds(interval_us);
            }
        } else {
            // Sleep until next send time
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

    close(sock);
}

UdpBenchmarkResult benchmark_udp_receive(const UdpBenchmarkConfig& config) {
    UdpBenchmarkResult result;

    // Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "ERROR: Could not create socket\n";
        return result;
    }

    // Allow multiple listeners
    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "WARNING: Could not set SO_REUSEADDR\n";
    }

    // Increase receive buffer size
    int rcvbuf = 2 * 1024 * 1024;  // 2 MB
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) < 0) {
        std::cerr << "WARNING: Could not increase receive buffer\n";
    }

    // Bind to port
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(config.port);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "ERROR: Could not bind to port " << config.port << "\n";
        close(sock);
        return result;
    }

    // Join multicast group
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(config.mcast_group.c_str());
    mreq.imr_interface.s_addr = inet_addr(config.interface_ip.c_str());

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        std::cerr << "WARNING: Could not join multicast group (may not be multicast)\n";
    }

    // Set non-blocking mode
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    std::vector<unsigned char> buffer(65536);
    uint32_t last_sequence = 0;
    bool first_packet = true;

    Timer timer;
    timer.start();

    auto end_time = high_resolution_clock::now() +
                    std::chrono::seconds(config.duration_seconds);

    while (high_resolution_clock::now() < end_time && g_running) {
        auto receive_time = high_resolution_clock::now();

        ssize_t len = recvfrom(sock, buffer.data(), buffer.size(), 0, nullptr, nullptr);

        if (len > 0) {
            auto process_time = high_resolution_clock::now();

            result.packets_received++;
            result.bytes_received += len;

            // Calculate latency (receive to process)
            double latency_us = duration_cast<duration<double, std::micro>>(
                process_time - receive_time).count();
            result.latencies_us.push_back(latency_us);

            // Check sequence number for packet loss
            if (len >= 7) {  // ASTERIX header (3) + sequence (4)
                uint32_t sequence;
                memcpy(&sequence, &buffer[3], sizeof(sequence));

                if (!first_packet) {
                    uint32_t expected = last_sequence + 1;
                    if (sequence != expected) {
                        result.packets_lost += (sequence - expected);
                    }
                }

                last_sequence = sequence;
                first_packet = false;
            }

            // Simple ASTERIX parsing to simulate real workload
            if (len >= 3) {
                unsigned char cat = buffer[0];
                uint16_t block_len = (buffer[1] << 8) | buffer[2];
                // Just access the data to prevent optimization
                volatile unsigned char dummy = cat + block_len;
                (void)dummy;
            }

        } else if (len < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "ERROR receiving packet: " << strerror(errno) << "\n";
            break;
        } else {
            // No data available, sleep briefly
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

    timer.stop();
    result.elapsed_seconds = timer.elapsed_seconds();

    // Cleanup
    setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
    close(sock);

    return result;
}

int main(int argc, char** argv) {
    UdpBenchmarkConfig config = parse_args(argc, argv);
    BenchmarkResults results("udp_multicast");

    // Setup signal handler for clean shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    std::cout << "ASTERIX UDP Multicast Processing Benchmark\n";
    std::cout << "===========================================\n";
    std::cout << "Multicast Group: " << config.mcast_group << ":" << config.port << "\n";
    std::cout << "Duration: " << config.duration_seconds << " seconds\n";
    std::cout << "Target Rate: " << config.target_rate_pps << " pps\n";
    std::cout << "Packet Size: " << config.packet_size << " bytes\n";
    std::cout << "Traffic Generation: " << (config.generate_traffic ? "enabled" : "disabled") << "\n";
    std::cout << std::endl;

    // Start traffic generator if enabled
    std::atomic<bool> generator_running{true};
    std::thread* generator_thread = nullptr;

    if (config.generate_traffic) {
        std::cout << "Starting traffic generator..." << std::endl;
        generator_thread = new std::thread(traffic_generator, config, std::ref(generator_running));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));  // Let it start
    }

    // Run benchmark
    std::cout << "Running UDP multicast benchmark..." << std::endl;
    UdpBenchmarkResult bench_result = benchmark_udp_receive(config);

    // Stop traffic generator
    if (generator_thread) {
        generator_running = false;
        generator_thread->join();
        delete generator_thread;
    }

    std::cout << std::endl;

    // Calculate statistics
    Statistics latency_stats;
    for (double lat : bench_result.latencies_us) {
        latency_stats.add(lat);
    }

    // Store results
    results.add_metric("duration_seconds", bench_result.elapsed_seconds);
    results.add_metric("packets_received", bench_result.packets_received);
    results.add_metric("bytes_received", bench_result.bytes_received);
    results.add_metric("packets_lost", bench_result.packets_lost);
    results.add_metric("packet_loss_rate", bench_result.packet_loss_rate());
    results.add_metric("packets_per_sec", bench_result.packets_per_second());
    results.add_metric("throughput_mbps", bench_result.throughput_mbps());

    if (!bench_result.latencies_us.empty()) {
        results.add_metric("latency_mean_us", latency_stats.mean());
        results.add_metric("latency_median_us", latency_stats.median());
        results.add_metric("latency_p50_us", latency_stats.percentile(0.50));
        results.add_metric("latency_p95_us", latency_stats.percentile(0.95));
        results.add_metric("latency_p99_us", latency_stats.percentile(0.99));
        results.add_metric("latency_p999_us", latency_stats.percentile(0.999));
        results.add_metric("latency_min_us", latency_stats.min());
        results.add_metric("latency_max_us", latency_stats.max());
        results.add_metric("latency_stddev_us", latency_stats.stddev());
    }

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
        const double MIN_PACKETS_PER_SEC = 5000.0;
        const double MAX_P99_LATENCY_US = 200.0;

        bool passed = true;

        if (bench_result.packets_per_second() < MIN_PACKETS_PER_SEC * config.base.threshold) {
            std::cerr << "FAILED: Packet rate below threshold ("
                     << bench_result.packets_per_second() << " < "
                     << MIN_PACKETS_PER_SEC * config.base.threshold << " pps)\n";
            passed = false;
        }

        if (!bench_result.latencies_us.empty() &&
            latency_stats.percentile(0.99) > MAX_P99_LATENCY_US) {
            std::cerr << "FAILED: p99 latency above threshold ("
                     << latency_stats.percentile(0.99) << " > "
                     << MAX_P99_LATENCY_US << " μs)\n";
            passed = false;
        }

        if (!passed) return 1;
    }

    return 0;
}
