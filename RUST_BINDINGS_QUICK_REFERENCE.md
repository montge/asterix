# Rust Bindings - Quick Reference Guide

## One-Page Summary

**What:** Add Rust language bindings to ASTERIX decoder
**Why:** Enable Rust developers to parse ATM surveillance data with type safety
**How:** CXX crate for safe C++/Rust FFI
**Timeline:** 12 weeks (can be 6 weeks with 2 engineers)
**Outcome:** Production crate on crates.io

---

## Technology Stack

| Component | Choice | Rationale |
|-----------|--------|-----------|
| **FFI Framework** | CXX 1.0 | Type-safe, compile-time checking, no boilerplate |
| **Build System** | Cargo + build.rs | Single source of truth, cross-platform |
| **Error Handling** | Result<T, AsterixError> | Idiomatic Rust, no panics |
| **Memory Model** | Owned Vec<AsterixRecord> | Simple ownership, no lifetime issues |
| **Testing** | Unit + Integration + Fuzzing | 90%+ coverage target |
| **Performance** | ~95% of C++ | FFI marshaling < 5% overhead |

---

## Implementation Timeline

```
Phase 1 (Weeks 1-2):   Foundation    ██░░░░░░░░
Phase 2 (Weeks 3-4):   Core API      ██████░░░░
Phase 3 (Weeks 5-6):   Advanced      ███████░░░
Phase 4 (Weeks 7-8):   Performance   ████████░░
Phase 5 (Weeks 9-10):  Testing/Docs  █████████░
Phase 6 (Weeks 11-12): Release       ██████████

Total: 12 weeks | MVP: Week 4 | Ready: Week 12
```

---

## API Design (Python → Rust Mapping)

### Basic Usage

```python
# Python
import asterix
data = open('file.pcap', 'rb').read()
records = asterix.parse(data)
```

```rust
// Rust
use asterix_decoder::parse;
let data = std::fs::read("file.pcap")?;
let records = parse(&data, Default::default())?;
```

### Core Functions

| Operation | Python | Rust |
|-----------|--------|------|
| **Parse** | `asterix.parse(data)` | `parse(data, opts)` |
| **Incremental** | `asterix.parse_with_offset(data, offset, count)` | `parse_with_offset(data, offset, count, opts)` |
| **Describe** | `asterix.describe(cat, item, field, value)` | `describe(cat, item, field, value)` |
| **Init** | `asterix.init(path)` | `Parser::new().add_category(path)` |

### Data Types

```rust
pub struct AsterixRecord {
    pub category: u8,
    pub length: u32,
    pub timestamp_ms: u64,
    pub crc: u32,
    pub hex_data: String,
    pub items: BTreeMap<String, DataItem>,
}

pub struct DataItem {
    pub description: Option<String>,
    pub fields: BTreeMap<String, ParsedValue>,
}

pub enum ParsedValue {
    Integer(i64),
    Float(f64),
    String(String),
    Boolean(bool),
    Bytes(Vec<u8>),
    Nested(Box<ParsedValue>),
}
```

---

## Phase Breakdown

### Phase 1: Foundation (Weeks 1-2)

**Deliverables:**
- ✓ Cargo.toml with metadata
- ✓ build.rs with C++ compilation
- ✓ CXX FFI declarations
- ✓ GitHub Actions CI/CD

**Key Files:**
```
asterix-rs/
├── Cargo.toml          (dependencies, metadata)
├── build.rs            (C++ build script)
└── src/
    └── ffi.rs          (CXX bridge)
```

**Effort:** 40 hours | Single engineer

---

### Phase 2: Core Parsing (Weeks 3-4)

**Deliverables:**
- ✓ `parse()` function (safe wrapper)
- ✓ Data type definitions
- ✓ Error handling
- ✓ 50+ unit tests

**Key Files:**
```
asterix-rs/src/
├── lib.rs              (public API)
├── parser.rs           (parse logic)
├── data_types.rs       (AsterixRecord, etc.)
└── error.rs            (AsterixError enum)
```

**Effort:** 50 hours | MVP delivered

---

### Phase 3: Advanced Features (Weeks 5-6)

**Deliverables:**
- ✓ Configuration management
- ✓ `describe()` function
- ✓ PCAP/HDLC format support
- ✓ Windows/macOS builds

**Key Files:**
```
asterix-rs/src/
├── config.rs           (custom categories)
├── formats.rs          (PCAP, HDLC, etc.)
└── tests/
    └── integration_tests.rs
```

**Effort:** 60 hours | Production-ready

---

### Phase 4: Performance (Weeks 7-8)

**Deliverables:**
- ✓ Benchmark suite
- ✓ Performance profiling
- ✓ Optimizations
- ✓ Memory validation

**Key Files:**
```
asterix-rs/
├── benches/
│   └── benchmarks.rs   (criterion benchmarks)
└── src/
    └── optimizations/  (perf improvements)
```

**Effort:** 55 hours | ~95% of C++ speed

---

### Phase 5: Testing & Docs (Weeks 9-10)

**Deliverables:**
- ✓ 90%+ test coverage
- ✓ Full rustdoc
- ✓ Migration guide
- ✓ Example code

**Key Files:**
```
asterix-rs/
├── docs/
│   ├── ARCHITECTURE.md
│   ├── MIGRATION_GUIDE.md
│   └── EXAMPLES.md
└── examples/
    ├── parse_pcap.rs
    ├── describe.rs
    └── incremental_parsing.rs
```

**Effort:** 50 hours | Feature-complete

---

### Phase 6: Release (Weeks 11-12)

**Deliverables:**
- ✓ crates.io publication
- ✓ docs.rs hosting
- ✓ Security audit
- ✓ Release notes

**Key Files:**
```
CHANGELOG.md
SECURITY.md
.github/workflows/publish.yml
```

**Effort:** 40 hours | Production deployment

---

## Key Design Decisions

### 1. Why CXX over Bindgen?

| Criterion | CXX | Bindgen |
|-----------|-----|---------|
| **Type safety** | ✓ Compile-time | Raw unsafe |
| **Ease of use** | Bridge DSL | Auto-generation |
| **Error handling** | Native Result<> | Manual conversion |
| **Performance** | Zero-cost | Zero-cost |

**Verdict:** CXX wins for complex C++, safety guarantees

### 2. Why Owned Data Model?

**Alternative: Zero-copy with lifetime parameters**
```rust
struct AsterixRecord<'a> {
    data: &'a [u8],  // Points into C++ memory
}
```

**Problem:** C++ deallocates before Rust uses data.
**Solution:** Copy data into Rust Vec at parse time.
**Trade-off:** One allocation vs. lifetime complexity.

### 3. Why Cargo Only (No CMake)?

**Alternative: Separate CMake builds**
- ✗ Complex integration
- ✗ Duplicate configuration
- ✗ Hard to maintain consistency

**Solution:** CMake called from build.rs
- ✓ Single entry point: `cargo build`
- ✓ Hermetic builds (reproducible)
- ✓ Cross-platform native

---

## Performance Targets

### Expected Performance

```
Input Size | Time     | vs C++ | vs Python
-----------|----------|--------|----------
512 B      | 50 μs    | 98%    | 10x faster
64 KB      | 700 μs   | 96%    | 5x faster
1 MB       | 12 ms    | 95%    | 3x faster
```

### Performance Budget

```
C++ parsing:        100% (baseline)
FFI marshaling:     2-5% (acceptable)
Rust overhead:      3-5% (allocations)
```

---

## Testing Strategy

### Coverage Goals

```
Unit Tests:           60 tests
Integration Tests:    40 tests
Property-based:       20 tests
Fuzzing:              1000+ seeds
Coverage Target:      90%+
```

### Test Categories

```
Unit:
  - Data type parsing
  - Error handling
  - Edge cases

Integration:
  - Sample PCAP files
  - Multi-category records
  - Incremental parsing

Fuzzing:
  - Invalid inputs
  - Truncated data
  - Malformed headers
```

---

## Project Structure

```
asterix/ (workspace root)
├── Cargo.toml (workspace)
├── src/ (C++ - unchanged)
├── asterix/ (Python - unchanged)
└── asterix-rs/ (NEW)
    ├── Cargo.toml
    ├── build.rs
    ├── src/
    │   ├── lib.rs
    │   ├── ffi.rs (unsafe FFI)
    │   ├── parser.rs (safe API)
    │   ├── data_types.rs
    │   ├── error.rs
    │   └── config.rs
    ├── tests/
    │   ├── integration_tests.rs
    │   ├── fixtures/
    │   │   ├── cat048.raw
    │   │   └── cat062.pcap
    │   └── property_tests.rs
    ├── examples/
    │   ├── parse_pcap.rs
    │   ├── describe.rs
    │   └── incremental.rs
    ├── benches/
    │   └── benchmarks.rs
    └── docs/
        ├── ARCHITECTURE.md
        ├── PERFORMANCE.md
        └── MIGRATION_GUIDE.md
```

---

## Risks & Mitigations

### Top 5 Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|-----------|
| C++ ABI breaking | Medium | High | Vendor C++ core |
| Memory safety | Low | Critical | Fuzzing + MSAN |
| Performance regression | Low | Medium | Benchmarks in CI |
| Windows build failure | Medium | Medium | Early CI testing |
| libexpat unavailable | Low | Medium | Static fallback |

### Risk Prevention

- **Week 1:** Test on all 3 platforms
- **Every PR:** Run fuzzer + MSAN/ASAN
- **Benchmarks:** >90% of C++ or blocked
- **Security:** cargo-audit on each build

---

## Success Criteria

### Must-Have (MVP)

- [ ] Compiles on Linux/macOS/Windows
- [ ] parse() and parse_with_offset() work
- [ ] No unsafe in public API
- [ ] 50+ passing tests

### Should-Have (v0.1)

- [ ] 90%+ test coverage
- [ ] Full documentation
- [ ] Benchmarks (95% of C++)
- [ ] crates.io published

### Nice-to-Have (v0.2+)

- [ ] Async streaming parser
- [ ] SIMD optimizations
- [ ] Memory pooling
- [ ] Wireshark integration

---

## Quick Start Commands

```bash
# Create workspace
cd asterix
cargo new asterix-rs --lib

# Build
cd asterix-rs
cargo build

# Test
cargo test --all-features

# Benchmark
cargo bench

# Check
cargo clippy
cargo fmt --check
cargo test --doc

# Publish (dry run)
cargo publish --dry-run

# Publish (real)
cargo publish --token $CRATES_IO_TOKEN
```

---

## Dependencies (Summary)

| Crate | Purpose | Phase |
|-------|---------|-------|
| cxx | FFI bridge | 1 |
| thiserror | Error types | 2 |
| serde | Serialization | 3 |
| criterion | Benchmarking | 4 |
| proptest | Property tests | 5 |
| cargo-fuzz | Fuzzing | 5 |

**Total:** 7 production + 6 dev dependencies

---

## Resource Estimates

### Single Engineer (12 weeks)

```
Foundation:  2 weeks × 40h = 80h
Core API:    2 weeks × 50h = 100h
Advanced:    2 weeks × 60h = 120h
Performance: 2 weeks × 55h = 110h
Testing:     2 weeks × 50h = 100h
Release:     2 weeks × 40h = 80h
────────────────────────────────
Total:      12 weeks × ~50h = 590h (~15 weeks @ 40h/week)
```

### Two Engineers (6 weeks)

```
Can run Phases in parallel:
- Phase 1-2: Engineer A (4 weeks)
- Phase 3-4: Engineer B (4 weeks in parallel)
- Phase 5-6: Both (2 weeks sequential)

Total: 6 weeks wall-clock time
```

---

## Next Steps (Immediate)

1. **Day 1:** Create `asterix-rs/` directory
2. **Day 2:** Set up Cargo.toml
3. **Day 3:** Implement build.rs
4. **Day 4:** Verify compilation
5. **Day 5:** Begin FFI declarations

**First Milestone:** Successful build on all 3 platforms (Week 1)

---

## Resources

**Documentation:**
- CXX Book: https://cxx.rs/
- Rust FFI: https://doc.rust-lang.org/nomicon/ffi.html
- ASTERIX Specs: http://www.eurocontrol.int/

**Tools:**
- GitHub Actions (CI/CD)
- cargo-fuzz (fuzzing)
- perf (profiling)
- valgrind (memory)
- cargo-audit (security)

**Community:**
- r/rust (general help)
- users.rust-lang.org (FFI questions)
- GitHub Discussions (project-specific)

---

## Document Guide

- **RUST_BINDINGS_IMPLEMENTATION_PLAN.md** - Full technical plan (1,665 lines)
- **RUST_BINDINGS_QUICK_REFERENCE.md** - This document (quick overview)

**For planning:** Use QUICK_REFERENCE
**For implementation:** Use FULL PLAN
**For management:** Use timeline & estimates above

---

**Version:** 1.0
**Created:** 2025-11-01
**Status:** Ready for Kickoff
