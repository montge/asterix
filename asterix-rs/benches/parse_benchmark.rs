//! Comprehensive benchmarks for ASTERIX Rust parser
//!
//! Compares Rust performance against baseline expectations and
//! measures performance across different data sizes and formats.
//!
//! Run with: cargo bench

use asterix::{parse, parse_with_offset, ParseOptions};
use criterion::{black_box, criterion_group, criterion_main, BenchmarkId, Criterion, Throughput};
use std::fs;
use std::path::PathBuf;

fn sample_data_path(filename: &str) -> PathBuf {
    let mut path = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    path.pop();
    path.push("install/sample_data");
    path.push(filename);
    path
}

/// Benchmark parsing of CAT048 raw data
fn bench_parse_cat048_raw(c: &mut Criterion) {
    let path = sample_data_path("cat048.raw");
    let data = fs::read(&path).expect("Failed to read cat048.raw");

    let mut group = c.benchmark_group("parse_cat048_raw");
    group.throughput(Throughput::Bytes(data.len() as u64));

    group.bench_function("default_options", |b| {
        b.iter(|| {
            let records = parse(black_box(&data), ParseOptions::default()).expect("Parse failed");
            black_box(records)
        })
    });

    group.bench_function("verbose_mode", |b| {
        let options = ParseOptions {
            verbose: true,
            filter_category: None,
            max_records: None,
        };
        b.iter(|| {
            let records = parse(black_box(&data), options.clone()).expect("Parse failed");
            black_box(records)
        })
    });

    group.finish();
}

/// Benchmark parsing of PCAP format (CAT062/065)
fn bench_parse_pcap_format(c: &mut Criterion) {
    let path = sample_data_path("cat_062_065.pcap");
    let data = fs::read(&path).expect("Failed to read PCAP");

    let mut group = c.benchmark_group("parse_pcap_format");
    group.throughput(Throughput::Bytes(data.len() as u64));

    group.bench_function("cat_062_065", |b| {
        b.iter(|| {
            let records = parse(black_box(&data), ParseOptions::default()).expect("Parse failed");
            black_box(records)
        })
    });

    group.finish();
}

/// Benchmark large PCAP file (CAT034/048)
fn bench_parse_large_pcap(c: &mut Criterion) {
    let path = sample_data_path("cat_034_048.pcap");
    let data = fs::read(&path).expect("Failed to read large PCAP");

    let mut group = c.benchmark_group("parse_large_pcap");
    group.throughput(Throughput::Bytes(data.len() as u64));
    group.sample_size(50); // Reduce sample size for large files

    group.bench_function("cat_034_048", |b| {
        b.iter(|| {
            let records = parse(black_box(&data), ParseOptions::default()).expect("Parse failed");
            black_box(records)
        })
    });

    group.finish();
}

/// Benchmark multicast PCAP (asterix.pcap)
fn bench_parse_multicast_pcap(c: &mut Criterion) {
    let path = sample_data_path("asterix.pcap");
    let data = fs::read(&path).expect("Failed to read asterix.pcap");

    let mut group = c.benchmark_group("parse_multicast_pcap");
    group.throughput(Throughput::Bytes(data.len() as u64));
    group.sample_size(50);

    group.bench_function("asterix_pcap", |b| {
        b.iter(|| {
            let records = parse(black_box(&data), ParseOptions::default()).expect("Parse failed");
            black_box(records)
        })
    });

    group.finish();
}

/// Benchmark incremental parsing
fn bench_incremental_parsing(c: &mut Criterion) {
    let path = sample_data_path("cat_034_048.pcap");
    let data = fs::read(&path).expect("Failed to read PCAP");

    let mut group = c.benchmark_group("incremental_parsing");
    group.throughput(Throughput::Bytes(data.len() as u64));

    // Benchmark different block sizes
    for blocks_per_iteration in [1, 5, 10, 50, 100] {
        group.bench_with_input(
            BenchmarkId::new("blocks", blocks_per_iteration),
            &blocks_per_iteration,
            |b, &blocks| {
                b.iter(|| {
                    let mut offset = 0;
                    let mut total_records = 0;

                    loop {
                        match parse_with_offset(
                            black_box(&data),
                            offset,
                            blocks,
                            ParseOptions::default(),
                        ) {
                            Ok(result) => {
                                total_records += result.records.len();
                                offset = result.bytes_consumed;

                                if result.remaining_blocks == 0 {
                                    break;
                                }
                            }
                            Err(_) => break,
                        }
                    }

                    black_box(total_records)
                })
            },
        );
    }

    group.finish();
}

/// Benchmark parsing with category filter
fn bench_parse_with_filter(c: &mut Criterion) {
    let path = sample_data_path("cat_062_065.pcap");
    let data = fs::read(&path).expect("Failed to read PCAP");

    let mut group = c.benchmark_group("parse_with_filter");
    group.throughput(Throughput::Bytes(data.len() as u64));

    group.bench_function("no_filter", |b| {
        let options = ParseOptions {
            verbose: false,
            filter_category: None,
            max_records: None,
        };
        b.iter(|| {
            let records = parse(black_box(&data), options.clone()).expect("Parse failed");
            black_box(records)
        })
    });

    group.bench_function("cat62_filter", |b| {
        let options = ParseOptions {
            verbose: false,
            filter_category: Some(62),
            max_records: None,
        };
        b.iter(|| {
            let records = parse(black_box(&data), options.clone()).expect("Parse failed");
            black_box(records)
        })
    });

    group.finish();
}

/// Benchmark parsing with max records limit
fn bench_parse_with_limit(c: &mut Criterion) {
    let path = sample_data_path("cat_034_048.pcap");
    let data = fs::read(&path).expect("Failed to read PCAP");

    let mut group = c.benchmark_group("parse_with_limit");
    group.throughput(Throughput::Bytes(data.len() as u64));

    for max_records in [1, 10, 100, 1000] {
        group.bench_with_input(
            BenchmarkId::new("max_records", max_records),
            &max_records,
            |b, &max| {
                let options = ParseOptions {
                    verbose: false,
                    filter_category: None,
                    max_records: Some(max),
                };
                b.iter(|| {
                    let records = parse(black_box(&data), options.clone()).expect("Parse failed");
                    black_box(records)
                })
            },
        );
    }

    group.finish();
}

/// Benchmark memory allocation patterns
fn bench_memory_allocation(c: &mut Criterion) {
    let path = sample_data_path("cat048.raw");
    let data = fs::read(&path).expect("Failed to read file");

    let mut group = c.benchmark_group("memory_allocation");

    // Benchmark repeated parsing (stress test for memory allocation)
    group.bench_function("repeated_parse_10x", |b| {
        b.iter(|| {
            for _ in 0..10 {
                let records =
                    parse(black_box(&data), ParseOptions::default()).expect("Parse failed");
                black_box(records);
            }
        })
    });

    group.finish();
}

/// Benchmark data size scaling
fn bench_data_size_scaling(c: &mut Criterion) {
    let mut group = c.benchmark_group("data_size_scaling");

    // Create synthetic ASTERIX data of various sizes
    let small_path = sample_data_path("cat048.raw");
    let medium_path = sample_data_path("cat_062_065.pcap");
    let large_path = sample_data_path("cat_034_048.pcap");

    let small_data = fs::read(&small_path).expect("Failed to read small file");
    let medium_data = fs::read(&medium_path).expect("Failed to read medium file");
    let large_data = fs::read(&large_path).expect("Failed to read large file");

    group.throughput(Throughput::Bytes(small_data.len() as u64));
    group.bench_function("small_48B", |b| {
        b.iter(|| {
            let records =
                parse(black_box(&small_data), ParseOptions::default()).expect("Parse failed");
            black_box(records)
        })
    });

    group.throughput(Throughput::Bytes(medium_data.len() as u64));
    group.bench_function("medium_255B", |b| {
        b.iter(|| {
            let records =
                parse(black_box(&medium_data), ParseOptions::default()).expect("Parse failed");
            black_box(records)
        })
    });

    group.throughput(Throughput::Bytes(large_data.len() as u64));
    group.bench_function("large_12KB", |b| {
        b.iter(|| {
            let records =
                parse(black_box(&large_data), ParseOptions::default()).expect("Parse failed");
            black_box(records)
        })
    });

    group.finish();
}

/// Benchmark error handling overhead
fn bench_error_handling(c: &mut Criterion) {
    let mut group = c.benchmark_group("error_handling");

    let valid_data = fs::read(sample_data_path("cat048.raw")).expect("Failed to read valid data");
    let invalid_data = b"Invalid ASTERIX data";

    group.bench_function("valid_data", |b| {
        b.iter(|| {
            let _ = parse(black_box(&valid_data), ParseOptions::default());
        })
    });

    group.bench_function("invalid_data", |b| {
        b.iter(|| {
            let _ = parse(black_box(invalid_data), ParseOptions::default());
        })
    });

    group.finish();
}

#[cfg(feature = "serde")]
fn bench_serialization(c: &mut Criterion) {
    use serde_json;

    let path = sample_data_path("cat048.raw");
    let data = fs::read(&path).expect("Failed to read file");
    let records = parse(&data, ParseOptions::default()).expect("Failed to parse");

    let mut group = c.benchmark_group("serialization");

    group.bench_function("to_json", |b| {
        b.iter(|| {
            let json = serde_json::to_string(black_box(&records)).expect("Serialization failed");
            black_box(json)
        })
    });

    let json = serde_json::to_string(&records).expect("Failed to serialize");

    group.bench_function("from_json", |b| {
        b.iter(|| {
            let records: Vec<asterix::AsterixRecord> =
                serde_json::from_str(black_box(&json)).expect("Deserialization failed");
            black_box(records)
        })
    });

    group.finish();
}

criterion_group!(
    benches,
    bench_parse_cat048_raw,
    bench_parse_pcap_format,
    bench_parse_large_pcap,
    bench_parse_multicast_pcap,
    bench_incremental_parsing,
    bench_parse_with_filter,
    bench_parse_with_limit,
    bench_memory_allocation,
    bench_data_size_scaling,
    bench_error_handling,
);

#[cfg(feature = "serde")]
criterion_group!(serde_benches, bench_serialization);

#[cfg(feature = "serde")]
criterion_main!(benches, serde_benches);

#[cfg(not(feature = "serde"))]
criterion_main!(benches);
