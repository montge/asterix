//! Zenoh Transport Benchmarks
//!
//! Measures performance characteristics of the Zenoh pub/sub transport:
//! - Latency: Time for publish → receive roundtrip
//! - Throughput: Messages per second at various payload sizes
//! - Connection time: Session establishment overhead
//!
//! Run with: cargo bench --features zenoh -- zenoh
//!
//! Note: These benchmarks require the zenoh feature and use peer-to-peer mode.

#![cfg(feature = "zenoh")]

use asterix::transport::zenoh::{ZenohConfig, ZenohPublisher, ZenohSubscriber};
use asterix::types::{AsterixRecord, DataItem, ParsedValue};
use criterion::{criterion_group, criterion_main, BenchmarkId, Criterion, Throughput};
use std::collections::BTreeMap;
use std::hint::black_box;
use std::time::Duration;
use tokio::runtime::Runtime;

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
    // Create a valid-looking ASTERIX header followed by payload
    let mut data = Vec::with_capacity(size);
    if size >= 3 {
        data.push(0x30); // Category 48
        data.push(((size >> 8) & 0xFF) as u8); // Length high byte
        data.push((size & 0xFF) as u8); // Length low byte
                                        // Fill rest with pattern
        data.extend(std::iter::repeat_n(0xAB, size.saturating_sub(3)));
    } else {
        data.extend(std::iter::repeat_n(0xAB, size));
    }
    data
}

/// Benchmark publisher connection establishment
fn bench_publisher_connect(c: &mut Criterion) {
    let rt = Runtime::new().unwrap();

    let mut group = c.benchmark_group("zenoh_publisher_connect");
    group.sample_size(20); // Connection benchmarks need fewer samples
    group.measurement_time(Duration::from_secs(10));

    group.bench_function("peer_to_peer", |b| {
        b.to_async(&rt).iter(|| async {
            let config = ZenohConfig::peer_to_peer();
            let publisher = ZenohPublisher::new(config)
                .await
                .expect("Failed to connect");
            black_box(&publisher);
            publisher.close().await.expect("Failed to close");
        })
    });

    group.finish();
}

/// Benchmark subscriber connection establishment
fn bench_subscriber_connect(c: &mut Criterion) {
    let rt = Runtime::new().unwrap();

    let mut group = c.benchmark_group("zenoh_subscriber_connect");
    group.sample_size(20);
    group.measurement_time(Duration::from_secs(10));

    group.bench_function("peer_to_peer", |b| {
        b.to_async(&rt).iter(|| async {
            let config = ZenohConfig::peer_to_peer();
            let subscriber = ZenohSubscriber::new(config, "asterix/**")
                .await
                .expect("Failed to connect");
            black_box(&subscriber);
            subscriber.close().await.expect("Failed to close");
        })
    });

    group.finish();
}

/// Benchmark raw publish throughput (no parsing overhead)
fn bench_publish_raw_throughput(c: &mut Criterion) {
    let rt = Runtime::new().unwrap();

    // Create publisher once for all benchmarks
    let publisher = rt.block_on(async {
        let config = ZenohConfig::peer_to_peer();
        ZenohPublisher::new(config)
            .await
            .expect("Failed to create publisher")
    });

    let mut group = c.benchmark_group("zenoh_publish_raw_throughput");
    group.measurement_time(Duration::from_secs(5));

    // Test different payload sizes
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
            b.to_async(&rt).iter(|| async {
                publisher
                    .publish_raw(48, black_box(data))
                    .await
                    .expect("Publish failed");
            })
        });
    }

    group.finish();

    // Cleanup
    rt.block_on(async {
        publisher.close().await.expect("Failed to close");
    });
}

/// Benchmark publishing with routing info (SAC/SIC)
fn bench_publish_with_routing(c: &mut Criterion) {
    let rt = Runtime::new().unwrap();

    let publisher = rt.block_on(async {
        let config = ZenohConfig::peer_to_peer();
        ZenohPublisher::new(config)
            .await
            .expect("Failed to create publisher")
    });

    let mut group = c.benchmark_group("zenoh_publish_with_routing");
    group.measurement_time(Duration::from_secs(5));

    let payload_sizes = [100, 1024, 4096];

    for size in payload_sizes {
        let payload = create_raw_payload(size);
        group.throughput(Throughput::Bytes(size as u64));

        group.bench_with_input(
            BenchmarkId::new("with_routing", size),
            &payload,
            |b, data| {
                b.to_async(&rt).iter(|| async {
                    publisher
                        .publish_raw_with_routing(48, 1, 2, black_box(data))
                        .await
                        .expect("Publish failed");
                })
            },
        );
    }

    group.finish();

    rt.block_on(async {
        publisher.close().await.expect("Failed to close");
    });
}

/// Benchmark publishing AsterixRecord (includes serialization)
fn bench_publish_record(c: &mut Criterion) {
    let rt = Runtime::new().unwrap();

    let publisher = rt.block_on(async {
        let config = ZenohConfig::peer_to_peer();
        ZenohPublisher::new(config)
            .await
            .expect("Failed to create publisher")
    });

    let mut group = c.benchmark_group("zenoh_publish_record");
    group.measurement_time(Duration::from_secs(5));

    let record_sizes = [(100, "small_100B"), (1024, "medium_1KB")];

    for (size, name) in record_sizes {
        let record = create_test_record(48, size);
        group.throughput(Throughput::Bytes(size as u64));

        group.bench_with_input(BenchmarkId::new("record", name), &record, |b, rec| {
            b.to_async(&rt).iter(|| async {
                publisher
                    .publish(black_box(rec))
                    .await
                    .expect("Publish failed");
            })
        });
    }

    group.finish();

    rt.block_on(async {
        publisher.close().await.expect("Failed to close");
    });
}

/// Benchmark pub/sub roundtrip latency
fn bench_pubsub_latency(c: &mut Criterion) {
    let rt = Runtime::new().unwrap();

    let mut group = c.benchmark_group("zenoh_pubsub_latency");
    group.sample_size(50);
    group.measurement_time(Duration::from_secs(15));

    let payload_sizes = [100, 1024];

    for size in payload_sizes {
        let payload = create_raw_payload(size);

        group.bench_with_input(BenchmarkId::new("roundtrip", size), &payload, |b, data| {
            b.to_async(&rt).iter_custom(|iters| {
                let data = data.clone();
                async move {
                    let config = ZenohConfig::peer_to_peer();

                    // Create publisher and subscriber
                    let publisher = ZenohPublisher::new(config.clone())
                        .await
                        .expect("Failed to create publisher");

                    let mut subscriber = ZenohSubscriber::new(config, "asterix/99/**")
                        .await
                        .expect("Failed to create subscriber");

                    // Allow subscription to establish
                    tokio::time::sleep(Duration::from_millis(50)).await;

                    let start = std::time::Instant::now();

                    for _ in 0..iters {
                        // Publish
                        publisher
                            .publish_raw_with_routing(99, 1, 2, &data)
                            .await
                            .expect("Publish failed");

                        // Receive with timeout
                        let _ = tokio::time::timeout(Duration::from_millis(100), subscriber.recv())
                            .await;
                    }

                    let elapsed = start.elapsed();

                    // Cleanup
                    let _ = publisher.close().await;
                    let _ = subscriber.close().await;

                    elapsed
                }
            })
        });
    }

    group.finish();
}

/// Benchmark multiple publishers to single subscriber (fan-in)
fn bench_fan_in(c: &mut Criterion) {
    let rt = Runtime::new().unwrap();

    let mut group = c.benchmark_group("zenoh_fan_in");
    group.sample_size(20);
    group.measurement_time(Duration::from_secs(10));

    let payload = create_raw_payload(100);

    for num_publishers in [2, 4] {
        group.bench_with_input(
            BenchmarkId::new("publishers", num_publishers),
            &num_publishers,
            |b, &count| {
                b.to_async(&rt).iter_custom(|iters| {
                    let payload = payload.clone();
                    async move {
                        let config = ZenohConfig::peer_to_peer();

                        // Create multiple publishers
                        let mut publishers = Vec::new();
                        for _ in 0..count {
                            let p = ZenohPublisher::new(config.clone())
                                .await
                                .expect("Failed to create publisher");
                            publishers.push(p);
                        }

                        let start = std::time::Instant::now();

                        for _ in 0..iters {
                            // Each publisher sends one message
                            for (i, publisher) in publishers.iter().enumerate() {
                                publisher
                                    .publish_raw_with_routing(48, i as u8, 1, &payload)
                                    .await
                                    .expect("Publish failed");
                            }
                        }

                        let elapsed = start.elapsed();

                        // Cleanup
                        for p in publishers {
                            let _ = p.close().await;
                        }

                        elapsed
                    }
                })
            },
        );
    }

    group.finish();
}

/// Benchmark message throughput (messages per second)
fn bench_message_throughput(c: &mut Criterion) {
    let rt = Runtime::new().unwrap();

    let mut group = c.benchmark_group("zenoh_message_throughput");
    group.throughput(Throughput::Elements(1)); // Per message
    group.measurement_time(Duration::from_secs(5));

    let payload = create_raw_payload(100);

    let publisher = rt.block_on(async {
        let config = ZenohConfig::peer_to_peer();
        ZenohPublisher::new(config)
            .await
            .expect("Failed to create publisher")
    });

    group.bench_function("messages_per_second", |b| {
        b.to_async(&rt).iter(|| async {
            publisher
                .publish_raw(48, black_box(&payload))
                .await
                .expect("Publish failed");
        })
    });

    group.finish();

    rt.block_on(async {
        publisher.close().await.expect("Failed to close");
    });
}

criterion_group!(
    zenoh_benches,
    bench_publisher_connect,
    bench_subscriber_connect,
    bench_publish_raw_throughput,
    bench_publish_with_routing,
    bench_publish_record,
    bench_pubsub_latency,
    bench_fan_in,
    bench_message_throughput,
);

criterion_main!(zenoh_benches);
