//! CAN Bus Transport Benchmarks
//!
//! These benchmarks measure the performance of CAN Bus transport operations.
//!
//! # Prerequisites
//!
//! Set up virtual CAN interface (vcan0):
//! ```bash
//! sudo modprobe vcan
//! sudo ip link add dev vcan0 type vcan
//! sudo ip link set up vcan0
//! ```
//!
//! # Running
//!
//! ```bash
//! cargo bench --features can --bench can_benchmark
//! ```

#![cfg(feature = "can")]

use criterion::{criterion_group, criterion_main, Criterion, Throughput};
use std::hint::black_box;

use asterix::transport::can::{CanConfig, CanPublisher};

fn bench_can_publish_small(c: &mut Criterion) {
    let config = CanConfig::new("vcan0").unwrap();
    let publisher = match CanPublisher::new(config) {
        Ok(p) => p,
        Err(_) => {
            eprintln!("Warning: vcan0 not available, skipping CAN benchmarks");
            return;
        }
    };

    let data = vec![0x30, 0x00, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05];

    let mut group = c.benchmark_group("can_publish");
    group.throughput(Throughput::Bytes(data.len() as u64));

    group.bench_function("publish_small_8bytes", |b| {
        b.iter(|| {
            publisher
                .publish_raw(black_box(48), black_box(&data))
                .unwrap();
        });
    });

    group.finish();
}

fn bench_can_publish_medium(c: &mut Criterion) {
    let config = CanConfig::new("vcan0").unwrap();
    let publisher = match CanPublisher::new(config) {
        Ok(p) => p,
        Err(_) => return,
    };

    let data = vec![0x42; 50]; // 50 bytes - requires ~8 CAN frames

    let mut group = c.benchmark_group("can_publish");
    group.throughput(Throughput::Bytes(data.len() as u64));

    group.bench_function("publish_medium_50bytes", |b| {
        b.iter(|| {
            publisher
                .publish_raw(black_box(62), black_box(&data))
                .unwrap();
        });
    });

    group.finish();
}

fn bench_can_publish_large(c: &mut Criterion) {
    let config = CanConfig::new("vcan0").unwrap();
    let publisher = match CanPublisher::new(config) {
        Ok(p) => p,
        Err(_) => return,
    };

    let data = vec![0x42; 200]; // 200 bytes - requires ~29 CAN frames

    let mut group = c.benchmark_group("can_publish");
    group.throughput(Throughput::Bytes(data.len() as u64));

    group.bench_function("publish_large_200bytes", |b| {
        b.iter(|| {
            publisher
                .publish_raw(black_box(62), black_box(&data))
                .unwrap();
        });
    });

    group.finish();
}

fn bench_can_fragmentation(c: &mut Criterion) {
    use asterix::transport::can::{CanConfig, CanPublisher};

    let config = CanConfig::new("vcan0").unwrap();
    let _publisher = match CanPublisher::new(config) {
        Ok(p) => p,
        Err(_) => return,
    };

    let mut group = c.benchmark_group("can_fragmentation");

    // Benchmark different data sizes
    for size in [10, 50, 100, 200, 500].iter() {
        let data = vec![0x42; *size];
        group.throughput(Throughput::Bytes(*size as u64));

        group.bench_with_input(
            criterion::BenchmarkId::from_parameter(size),
            &data,
            |b, data| {
                b.iter(|| {
                    // Just benchmark the fragmentation logic
                    // (actual implementation would call internal fragment_data function)
                    let payload_size = 7; // Classic CAN
                    let mut _fragment_count = 0;
                    let mut offset = 0;
                    while offset < data.len() {
                        let remaining = data.len() - offset;
                        let _chunk_size = remaining.min(payload_size);
                        offset += payload_size;
                        _fragment_count += 1;
                    }
                });
            },
        );
    }

    group.finish();
}

fn bench_can_config_creation(c: &mut Criterion) {
    let mut group = c.benchmark_group("can_config");

    group.bench_function("config_new", |b| {
        b.iter(|| {
            let _config = CanConfig::new(black_box("vcan0")).unwrap();
        });
    });

    group.bench_function("config_with_fd", |b| {
        b.iter(|| {
            let _config = CanConfig::with_fd(black_box("vcan0")).unwrap();
        });
    });

    group.bench_function("config_with_timeout", |b| {
        b.iter(|| {
            let config = CanConfig::new(black_box("vcan0")).unwrap();
            let _config = config.with_timeout(black_box(2000));
        });
    });

    group.finish();
}

criterion_group!(
    benches,
    bench_can_config_creation,
    bench_can_publish_small,
    bench_can_publish_medium,
    bench_can_publish_large,
    bench_can_fragmentation,
);

criterion_main!(benches);
