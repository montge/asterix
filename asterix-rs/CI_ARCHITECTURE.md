# Rust CI/CD Architecture

## Workflow Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                    Rust Bindings CI/CD Pipeline                  │
│                  .github/workflows/rust-ci.yml                   │
└─────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
        ┌─────────────────────────────────────────────┐
        │         ON: push, pull_request, manual      │
        └─────────────────────────────────────────────┘
                                  │
        ┌─────────────────────────┴──────────────────────────┐
        │                                                     │
        ▼                                                     ▼
┌───────────────────┐                              ┌──────────────────┐
│   TEST (Matrix)   │                              │ QUALITY CHECKS   │
│ ──────────────── │                              │ ────────────────│
│ • 3 OS (L,M,W)    │                              │ • Format Check   │
│ • 2 Rust (S,N)    │                              │ • Clippy -D      │
│ • 6 total jobs    │                              │ • Doc Build      │
│                   │                              │ • Cargo Audit    │
│ CHECKS:           │                              │ • Outdated Deps  │
│ ✅ cargo build     │                              │ • Unused Deps    │
│ ✅ cargo test      │                              │                  │
│ ✅ cargo clippy    │                              │ STATUS: CRITICAL │
│ ✅ cargo fmt       │                              │ Fails → CI fails │
│                   │                              └──────────────────┘
│ STATUS: CRITICAL  │
│ Fails → CI fails  │
└───────────────────┘
        │
        ├─────────────────────┬──────────────────────┬────────────────┐
        ▼                     ▼                      ▼                ▼
┌──────────────┐    ┌──────────────┐      ┌──────────────┐  ┌──────────────┐
│  COVERAGE    │    │  COVERAGE    │      │  SECURITY    │  │ MEMORY SAFETY│
│ (Tarpaulin)  │    │ (LLVM-COV)   │      │              │  │  (Valgrind)  │
│──────────────│    │──────────────│      │──────────────│  │──────────────│
│ • LLVM engine│    │ • Native LLVM│      │ • cargo-audit│  │ • Valgrind   │
│ • HTML report│    │ • HTML report│      │ • cargo-deny │  │ • Leak check │
│ • XML (Cov.) │    │ • JSON report│      │ • Advisories │  │ • FFI safety │
│ • JSON report│    │ • LCOV format│      │ • Licenses   │  │              │
│ • LCOV format│    │              │      │ • Bans       │  │ STATUS: WARN │
│              │    │              │      │              │  │ FFI issues OK│
│ THRESHOLD:   │    │ THRESHOLD:   │      │ STATUS: WARN │  └──────────────┘
│ ❌ Fail < 80% │    │ ❌ Fail < 80% │      │ Advisories=❌│
│ ⚠️  Warn < 90%│    │ ⚠️  Warn < 90%│      │ Others=⚠️    │
│              │    │              │      └──────────────┘
│ CODECOV:     │    │ CODECOV:     │
│ rust,        │    │ rust,        │
│ tarpaulin    │    │ llvm-cov     │
│              │    │              │
│ STATUS: CRIT.│    │ STATUS: CRIT.│
│ Fails → CI   │    │ Fails → CI   │
│ (one must ✅)│    │ (one must ✅)│
└──────────────┘    └──────────────┘
        │                   │
        ├───────────────────┴──────────────┬────────────────┬──────────────┐
        ▼                                  ▼                ▼              ▼
┌──────────────┐                  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│  BENCHMARKS  │                  │     DOCS     │ │ CROSS-COMPILE│ │     MSRV     │
│──────────────│                  │──────────────│ │──────────────│ │──────────────│
│ • Performance│                  │ • rustdoc    │ │ • x86_64 Lin.│ │ • Rust 1.70  │
│ • Regression │                  │ • HTML gen.  │ │ • aarch64 Lin│ │ • Min. check │
│ • Artifact   │                  │ • Artifact   │ │ • x86_64 macOS│ │              │
│              │                  │              │ │ • aarch64 mac│ │ STATUS: WARN │
│ STATUS: WARN │                  │ STATUS: WARN │ │              │ │ Informational│
│ Informational│                  │ Informational│ │ STATUS: WARN │ └──────────────┘
└──────────────┘                  └──────────────┘ │ Informational│
                                                   └──────────────┘
        │                                                  │
        └──────────────────────┬───────────────────────────┘
                               ▼
                  ┌──────────────────────────┐
                  │   PERFORMANCE COMPARE    │
                  │ ─────────────────────── │
                  │ • Rust vs Python         │
                  │ • Benchmark comparison   │
                  │                          │
                  │ STATUS: WARN             │
                  │ Informational            │
                  └──────────────────────────┘
                               │
                               ▼
                  ┌──────────────────────────┐
                  │  RUST BUILD SUMMARY      │
                  │ ─────────────────────── │
                  │ Requires ALL jobs        │
                  │                          │
                  │ CRITICAL CHECKS:         │
                  │ • test ✅                 │
                  │ • quality-checks ✅       │
                  │ • coverage (1 of 2) ✅   │
                  │                          │
                  │ IF ALL ✅ → BUILD PASS   │
                  │ IF ANY ❌ → BUILD FAIL   │
                  └──────────────────────────┘
```

## Job Dependencies Graph

```
test (6 jobs) ────────┐
                      │
quality-checks ───────┤
                      │
coverage-tarpaulin ───┤
                      │
coverage-llvm-cov ────┤
                      │
benchmark ────────────┤
                      ├──→ rust-build-summary
security ─────────────┤
                      │
memory-safety ────────┤
                      │
docs ─────────────────┤
                      │
cross-compile ────────┤
                      │
msrv ─────────────────┤
                      │
perf-compare ─────────┘
```

## Coverage Flow

```
┌─────────────────────────────────────────────────────────┐
│                    Coverage Pipeline                     │
└─────────────────────────────────────────────────────────┘
                          │
         ┌────────────────┴────────────────┐
         ▼                                 ▼
┌──────────────────┐           ┌──────────────────┐
│   TARPAULIN      │           │    LLVM-COV      │
│   (Primary)      │           │  (Alternative)   │
└──────────────────┘           └──────────────────┘
         │                                 │
         ├─ Run Tests                      ├─ Run Tests
         ├─ Collect Coverage               ├─ Collect Coverage
         ├─ Generate Reports:              ├─ Generate Reports:
         │  • HTML                          │  • HTML
         │  • XML (Cobertura)               │  • JSON
         │  • JSON                          │  • LCOV
         │  • LCOV                          │
         │                                 │
         ├─ Check Threshold:               ├─ Check Threshold:
         │  • Fail if < 80%                 │  • Fail if < 80%
         │  • Warn if < 90%                 │  • Warn if < 90%
         │                                 │
         ├─ Upload Artifact                ├─ Upload Artifact
         │  (30 days)                       │  (30 days)
         │                                 │
         └─ Upload to Codecov              └─ Upload to Codecov
            flags: rust,tarpaulin             flags: rust,llvm-cov
                          │                                 │
                          └────────────┬────────────────────┘
                                       ▼
                          ┌────────────────────────┐
                          │   CODECOV DASHBOARD    │
                          │ ──────────────────────│
                          │ • Combined view        │
                          │ • Coverage trends      │
                          │ • PR comments          │
                          │ • Badges               │
                          │ • 90% target           │
                          │ • 80% minimum          │
                          └────────────────────────┘
```

## Quality Checks Flow

```
┌─────────────────────────────────────────────────────────┐
│                  Quality Checks Pipeline                 │
└─────────────────────────────────────────────────────────┘
                          │
         ┌────────────────┼────────────────┐
         ▼                ▼                ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│   FORMAT     │  │   CLIPPY     │  │  DOC BUILD   │
│──────────────│  │──────────────│  │──────────────│
│ cargo fmt    │  │ cargo clippy │  │ cargo doc    │
│ --check      │  │ --all        │  │ --all-feat.  │
│              │  │ -D warnings  │  │ --deny warn. │
│              │  │              │  │              │
│ FAIL: ❌     │  │ FAIL: ❌      │  │ FAIL: ❌      │
│ Unformatted  │  │ Any warnings │  │ Doc warnings │
└──────────────┘  └──────────────┘  └──────────────┘
         │                │                │
         └────────────────┼────────────────┘
                          │
         ┌────────────────┼────────────────┐
         ▼                ▼                ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│   AUDIT      │  │  OUTDATED    │  │    UDEPS     │
│──────────────│  │──────────────│  │──────────────│
│ cargo audit  │  │ cargo        │  │ cargo +night │
│ cargo deny   │  │ outdated     │  │ udeps        │
│ advisories   │  │              │  │              │
│              │  │              │  │              │
│ WARN: ⚠️     │  │ WARN: ⚠️      │  │ WARN: ⚠️      │
│ Log issues   │  │ Log updates  │  │ Log unused   │
└──────────────┘  └──────────────┘  └──────────────┘
                          │
                          ▼
                  ┌──────────────┐
                  │  ALL PASS?   │
                  └──────────────┘
                       │      │
                  YES  │      │  NO
                       ▼      ▼
                     ✅      ❌
                   PASS    FAIL
```

## Platform Matrix

```
┌─────────────────────────────────────────────────────────┐
│                  Test Matrix (6 jobs)                    │
└─────────────────────────────────────────────────────────┘

         Ubuntu          macOS          Windows
         ──────          ─────          ───────
Stable:   ✅              ✅              ✅
Nightly:  ✅              ⚠️              ⚠️
                    (reduced matrix)

Each job runs:
1. cargo build --all-features
2. cargo test --all-features
3. cargo clippy --all-features --all-targets -- -D warnings
4. cargo fmt -- --check

Dependencies per platform:
• Linux:   libexpat-dev (apt)
• macOS:   expat (brew)
• Windows: expat (vcpkg)
```

## Artifact Flow

```
┌─────────────────────────────────────────────────────────┐
│                    Artifact Generation                   │
└─────────────────────────────────────────────────────────┘

coverage-tarpaulin
    │
    ├─→ coverage-tarpaulin-reports/
    │       ├─ index.html (HTML report)
    │       ├─ cobertura.xml (Codecov)
    │       ├─ tarpaulin-report.json
    │       └─ lcov.info
    │
    └─→ Retention: 30 days

coverage-llvm-cov
    │
    ├─→ coverage-llvm-cov-reports/
    │       ├─ html/index.html
    │       ├─ coverage.json
    │       └─ lcov.info
    │
    └─→ Retention: 30 days

benchmark
    │
    ├─→ benchmark-results/
    │       └─ benchmark-results.txt
    │
    └─→ Retention: 30 days

docs
    │
    ├─→ rust-docs/
    │       └─ target/doc/**
    │
    └─→ Retention: 14 days

All artifacts downloadable from:
GitHub Actions → Workflow Run → Artifacts section
```

## Decision Tree

```
┌─────────────────────────────────────────────────────────┐
│                  Should Build Pass?                      │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
                ┌──────────────────┐
                │  test PASSED?    │
                └──────────────────┘
                   │            │
                 YES           NO
                   │            └──→ ❌ FAIL BUILD
                   ▼
           ┌──────────────────┐
           │ quality-checks   │
           │    PASSED?       │
           └──────────────────┘
                   │            │
                 YES           NO
                   │            └──→ ❌ FAIL BUILD
                   ▼
           ┌──────────────────┐
           │ coverage-tarp OR │
           │ coverage-llvm    │
           │    PASSED?       │
           └──────────────────┘
                   │            │
                 YES           NO
                   │            └──→ ❌ FAIL BUILD
                   ▼
                ┌──────┐
                │ ✅    │
                │ PASS │
                └──────┘

Optional jobs (don't block):
• benchmark
• security (partial)
• memory-safety
• docs
• cross-compile
• msrv
• perf-compare
```

## Coverage Threshold Logic

```
Coverage Percentage
       │
       ▼
   < 80%  ─────────→ ❌ FAIL BUILD
       │              "Coverage {X}% below 80% minimum"
       │
  80-90% ─────────→ ⚠️  WARN + PASS
       │              "Coverage {X}% below 90% target"
       │
   ≥ 90% ─────────→ ✅ PASS
                      "Coverage {X}% meets 90% target"

Applied to both:
• coverage-tarpaulin
• coverage-llvm-cov

At least ONE must pass for build to succeed
```

## Integration with Repository

```
Repository Settings
    │
    ├─→ Secrets
    │   └─ CODECOV_TOKEN (required for uploads)
    │
    ├─→ Branch Protection
    │   ├─ Require status checks:
    │   │  ├─ Test (all matrix jobs)
    │   │  ├─ Quality Checks
    │   │  ├─ Coverage (one of two)
    │   │  └─ Build Summary
    │   └─ Require PR reviews
    │
    └─→ Actions
        ├─ Workflow permissions: read
        └─ Cache: enabled (cargo registry/build)

README.md
    ├─→ Badges
    │   ├─ CI Status
    │   ├─ Codecov
    │   ├─ License
    │   └─ Rust Version
    └─→ Quality Standards section

Codecov Dashboard
    ├─→ Coverage trends
    ├─→ PR comments
    ├─→ File coverage
    └─→ Flag comparison (tarpaulin vs llvm-cov)
```

## Files and Configuration

```
asterix-rs/
    ├─ .tarpaulin.toml          # Tarpaulin config
    ├─ codecov.yml              # Codecov config
    ├─ COVERAGE.md              # Coverage guide
    ├─ QUICK_START_COVERAGE.md  # Quick reference
    ├─ RUST_CI_ENHANCEMENTS.md  # Implementation doc
    └─ README.md                # Badges + standards

.github/workflows/
    └─ rust-ci.yml              # CI/CD pipeline

Configuration hierarchy:
1. .tarpaulin.toml → tarpaulin CLI → coverage generation
2. codecov.yml → Codecov → coverage analysis
3. rust-ci.yml → GitHub Actions → CI/CD orchestration
```

## Summary

**Jobs**: 12 total
- **Critical**: 3 (test, quality-checks, coverage)
- **Recommended**: 2 (security, memory-safety)
- **Optional**: 7 (benchmark, docs, cross-compile, msrv, perf-compare)

**Coverage Tools**: 2
- Primary: cargo-tarpaulin (LLVM engine)
- Alternative: cargo-llvm-cov (cross-validation)

**Quality Gates**: 6
- Format, Clippy, Docs, Audit, Outdated, Unused deps

**Platforms**: 3 + 4 cross-compile targets
- Native: Linux, macOS, Windows
- Cross: x86_64/aarch64 Linux/Darwin

**Artifacts**: 4 types
- Coverage reports (30 days)
- Benchmark results (30 days)
- Documentation (14 days)
- All downloadable from Actions

**Enforcement**:
- Hard failures: test, quality, coverage < 80%
- Soft warnings: coverage < 90%, outdated deps, security advisories
- Informational: benchmarks, cross-compile, MSRV

**Target**: 90% coverage, zero warnings, full security compliance

---

**Last Updated**: 2025-11-01
**Maintained By**: asterix-rs team
