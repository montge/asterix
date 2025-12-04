//! DDS Transport Benchmarks
//!
//! Measures performance characteristics of the DDS pub/sub transport:
//! - Connection time: Domain participant and writer/reader creation
//! - Throughput: Messages per second at various payload sizes
//! - Latency: Publish → receive roundtrip time
//!
//! Run with: cargo bench --features dds -- dds
//!
//! Note: These benchmarks require the dds feature.

#![cfg(feature = "dds")]

use asterix::transport::dds::{DdsConfig, DdsPublisher, DdsSubscriber, Reliability};
use asterix::types::{AsterixRecord, DataItem, ParsedValue};
use criterion::{criterion_group, criterion_main, BenchmarkId, Criterion, Throughput};
use std::collections::BTreeMap;
use std::hint::black_box;
use std::thread;
use std::time::Duration;

/// Create a test AsterixRecord with given size characteristics
fn create_test_record(category: u8, payload_size: usize) -> AsterixRecord {
    let mut items = BTreeMap::new();
    let mut fields = BTreeMap::new();
    fields.insert("SAC".to_string(), ParsedValue::Integer(1));
    fields.insert("SIC".to_string(), ParsedValue::Integer(2));
    items.insert(
        format!("I{category:03}/010"),
        DataItem {
            description: Some("Data Source Identifier".to_string()),
            fields,
        },
    );

    // Create hex_data of approximate size
    let hex_data = "AB".repeat(payload_size / 2);

    AsterixRecord {
        category,
        length: payload_size as u32,
        timestamp_ms: 0,
        hex_data,
        items,
        crc: 0,
    }
}

/// Create raw ASTERIX-like payload of given size
fn create_raw_payload(size: usize) -> Vec<u8> {
    let mut data = Vec::with_capacity(size);
    if size >= 3 {
        data.push(0x30); // Category 48
        data.push(((size >> 8) & 0xFF) as u8);
        data.push((size & 0xFF) as u8);
        data.extend(std::iter::repeat_n(0xAB, size.saturating_sub(3)));
    } else {
        data.extend(std::iter::repeat_n(0xAB, size));
    }
    data
}

/// Benchmark publisher creation
fn bench_publisher_create(c: &mut Criterion) {
    let mut group = c.benchmark_group("dds_publisher_create");
    group.sample_size(20);
    group.measurement_time(Duration::from_secs(10));

    group.bench_function("default", |b| {
        b.iter(|| {
            let config = DdsConfig::default();
            let publisher = DdsPublisher::new(config).expect("Failed to create publisher");
            black_box(&publisher);
        })
    });

    group.bench_function("best_effort", |b| {
        b.iter(|| {
            let config = DdsConfig::best_effort();
            let publisher = DdsPublisher::new(config).expect("Failed to create publisher");
            black_box(&publisher);
        })
    });

    group.finish();
}

/// Benchmark subscriber creation
fn bench_subscriber_create(c: &mut Criterion) {
    let mut group = c.benchmark_group("dds_subscriber_create");
    group.sample_size(20);
    group.measurement_time(Duration::from_secs(10));

    group.bench_function("default", |b| {
        b.iter(|| {
            let config = DdsConfig::default();
            let subscriber = DdsSubscriber::new(config, "asterix_bench_sub")
                .expect("Failed to create subscriber");
            black_box(&subscriber);
        })
    });

    group.finish();
}

/// Benchmark raw publish throughput (no serialization overhead)
fn bench_publish_raw_throughput(c: &mut Criterion) {
    let config = DdsConfig::best_effort();
    let publisher = DdsPublisher::new(config).expect("Failed to create publisher");

    let mut group = c.benchmark_group("dds_publish_raw_throughput");
    group.measurement_time(Duration::from_secs(5));

    let payload_sizes = [
        (100, "small_100B"),
        (1024, "medium_1KB"),
        (4096, "large_4KB"),
        (65536, "xlarge_64KB"),
    ];

    for (size, name) in payload_sizes {
        let payload = create_raw_payload(size);
        group.throughput(Throughput::Bytes(size as u64));

        group.bench_with_input(BenchmarkId::new("raw", name), &payload, |b, data| {
            b.iter(|| {
                publisher
                    .publish_raw(48, black_box(data))
                    .expect("Publish failed");
            })
        });
    }

    group.finish();
}

/// Benchmark publishing with routing info (SAC/SIC)
fn bench_publish_with_routing(c: &mut Criterion) {
    let config = DdsConfig::best_effort();
    let publisher = DdsPublisher::new(config).expect("Failed to create publisher");

    let mut group = c.benchmark_group("dds_publish_with_routing");
    group.measurement_time(Duration::from_secs(5));

    let payload_sizes = [100, 1024, 4096];

    for size in payload_sizes {
        let payload = create_raw_payload(size);
        group.throughput(Throughput::Bytes(size as u64));

        group.bench_with_input(
            BenchmarkId::new("with_routing", size),
            &payload,
            |b, data| {
                b.iter(|| {
                    publisher
                        .publish_raw_with_routing(48, 1, 2, black_box(data))
                        .expect("Publish failed");
                })
            },
        );
    }

    group.finish();
}

/// Benchmark publishing AsterixRecord (includes serialization)
fn bench_publish_record(c: &mut Criterion) {
    let config = DdsConfig::best_effort();
    let publisher = DdsPublisher::new(config).expect("Failed to create publisher");

    let mut group = c.benchmark_group("dds_publish_record");
    group.measurement_time(Duration::from_secs(5));

    let record_sizes = [(100, "small_100B"), (1024, "medium_1KB")];

    for (size, name) in record_sizes {
        let record = create_test_record(48, size);
        group.throughput(Throughput::Bytes(size as u64));

        group.bench_with_input(BenchmarkId::new("record", name), &record, |b, rec| {
            b.iter(|| {
                publisher.publish(black_box(rec)).expect("Publish failed");
            })
        });
    }

    group.finish();
}

/// Benchmark pub/sub roundtrip latency
fn bench_pubsub_latency(c: &mut Criterion) {
    let mut group = c.benchmark_group("dds_pubsub_latency");
    group.sample_size(20);
    group.measurement_time(Duration::from_secs(15));

    let topic_name = "asterix_bench_latency";
    let payload_sizes = [100, 1024];

    for size in payload_sizes {
        let payload = create_raw_payload(size);

        group.bench_with_input(BenchmarkId::new("roundtrip", size), &payload, |b, data| {
            b.iter_custom(|iters| {
                let config = DdsConfig::best_effort();

                let publisher =
                    DdsPublisher::new(config.clone()).expect("Failed to create publisher");
                let mut subscriber =
                    DdsSubscriber::new(config, topic_name).expect("Failed to create subscriber");

                // Allow DDS discovery
                thread::sleep(Duration::from_millis(500));

                let start = std::time::Instant::now();

                for _ in 0..iters {
                    publisher.publish_raw(48, data).expect("Publish failed");

                    // Try to receive (may not always succeed due to DDS timing)
                    let _ = subscriber.recv_timeout(Duration::from_millis(100));
                }

                start.elapsed()
            })
        });
    }

    group.finish();
}

/// Benchmark message throughput (messages per second)
fn bench_message_throughput(c: &mut Criterion) {
    let config = DdsConfig {
        reliability: Reliability::BestEffort,
        ..Default::default()
    };
    let publisher = DdsPublisher::new(config).expect("Failed to create publisher");

    let mut group = c.benchmark_group("dds_message_throughput");
    group.measurement_time(Duration::from_secs(5));
    group.throughput(Throughput::Elements(1)); // 1 message per iteration

    let payload = create_raw_payload(100);

    group.bench_function("msgs_per_sec", |b| {
        b.iter(|| {
            publisher
                .publish_raw(48, black_box(&payload))
                .expect("Publish failed");
        })
    });

    group.finish();
}

/// Benchmark different QoS configurations
fn bench_qos_configurations(c: &mut Criterion) {
    let mut group = c.benchmark_group("dds_qos_configurations");
    group.measurement_time(Duration::from_secs(5));

    let payload = create_raw_payload(1024);
    group.throughput(Throughput::Bytes(1024));

    // Best effort
    {
        let config = DdsConfig::best_effort();
        let publisher = DdsPublisher::new(config).expect("Failed to create publisher");

        group.bench_with_input(
            BenchmarkId::new("publish", "best_effort"),
            &payload,
            |b, data| {
                b.iter(|| {
                    publisher
                        .publish_raw(48, black_box(data))
                        .expect("Publish failed");
                })
            },
        );
    }

    // Reliable
    {
        let config = DdsConfig::reliable();
        let publisher = DdsPublisher::new(config).expect("Failed to create publisher");

        group.bench_with_input(
            BenchmarkId::new("publish", "reliable"),
            &payload,
            |b, data| {
                b.iter(|| {
                    publisher
                        .publish_raw(48, black_box(data))
                        .expect("Publish failed");
                })
            },
        );
    }

    group.finish();
}

criterion_group!(
    name = dds_benches;
    config = Criterion::default()
        .significance_level(0.1)
        .sample_size(50)
        .warm_up_time(Duration::from_secs(2))
        .measurement_time(Duration::from_secs(5));
    targets =
        bench_publisher_create,
        bench_subscriber_create,
        bench_publish_raw_throughput,
        bench_publish_with_routing,
        bench_publish_record,
        bench_pubsub_latency,
        bench_message_throughput,
        bench_qos_configurations
);

criterion_main!(dds_benches);
