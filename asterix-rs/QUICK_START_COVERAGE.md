# Quick Start: Coverage Testing for asterix-rs

This guide gets you running coverage tests in under 5 minutes.

## One-Time Setup

```bash
# Install coverage tools
cargo install cargo-tarpaulin
cargo install cargo-llvm-cov
rustup component add llvm-tools-preview
```

## Run Coverage (Tarpaulin - Primary Tool)

```bash
cd asterix-rs

# Generate coverage with HTML report
cargo tarpaulin --all-features --workspace --out Html --output-dir coverage

# Open the report
open coverage/index.html  # macOS
xdg-open coverage/index.html  # Linux
start coverage/index.html  # Windows
```

**What you'll see:**
- Overall coverage percentage
- Per-file breakdown
- Highlighted uncovered lines (red = uncovered, green = covered)

## Run Coverage (LLVM-COV - Alternative)

```bash
cd asterix-rs

# Generate coverage with HTML report
cargo llvm-cov --all-features --workspace --html

# Open the report
open target/llvm-cov/html/index.html
```

## Check Coverage Threshold

```bash
cd asterix-rs

# Run with threshold check (fails if < 80%)
cargo tarpaulin --all-features --workspace --fail-under 80
```

**Expected output:**
- ✅ Coverage ≥ 80%: Build succeeds
- ❌ Coverage < 80%: Build fails with error

## CI/CD Pipeline

Coverage runs automatically on:
- Every push to master/develop/feature branches
- Every pull request
- Manual workflow dispatch

**Coverage jobs:**
1. `coverage-tarpaulin`: Primary coverage (fails build if < 80%)
2. `coverage-llvm-cov`: Secondary coverage (cross-validation)

**Artifacts:**
- HTML reports available for 30 days
- Download from Actions → Workflow Run → Artifacts

## Coverage Targets

- **Minimum**: 80% per module (CI fails below this)
- **Target**: 90% overall (warning if not met)
- **PR requirement**: New code should have ≥ 80% coverage

## Quick Checks Before Push

```bash
# Run all quality checks
cd asterix-rs

cargo fmt -- --check                                          # Format check
cargo clippy --all-features --all-targets -- -D warnings     # Lint check
cargo test --all-features                                     # Run tests
cargo tarpaulin --all-features --workspace --out Html         # Coverage

# If all pass, you're good to push!
```

## Interpreting HTML Reports

### Overall Summary (top of page)
```
Coverage: 85.3%
Lines: 1234 / 1447
```
- This is your overall coverage percentage
- Must be ≥ 80% for CI to pass

### File List
- Files sorted by coverage (lowest first by default)
- Click filename to see line-by-line coverage
- Focus on files with < 80% coverage

### Line-by-Line View
- **Green**: Line executed during tests ✅
- **Red**: Line never executed ❌
- **Yellow**: Partially covered (branches) ⚠️
- **Gray**: Non-executable (comments, braces)

## Improving Coverage

### Step 1: Find Gaps
```bash
cargo tarpaulin --all-features --workspace --out Html --output-dir coverage
open coverage/index.html
```
- Look for red lines
- Identify uncovered functions/modules

### Step 2: Write Tests
```rust
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_uncovered_function() {
        // Add test for previously uncovered code
        assert_eq!(uncovered_function(), expected_value);
    }
}
```

### Step 3: Verify Improvement
```bash
cargo tarpaulin --all-features --workspace
```
- Check that coverage increased
- Ensure new code is now green in HTML report

### Step 4: Commit
```bash
git add tests/
git commit -m "Add tests for XYZ module - coverage 85% → 92%"
```

## Troubleshooting

### "Coverage seems wrong"
- Compare tarpaulin vs llvm-cov results
- Check that all features are enabled (`--all-features`)
- Verify tests are actually running (`cargo test --all-features`)

### "Tests timeout"
- Increase timeout: `cargo tarpaulin --timeout 600`
- Or in `.tarpaulin.toml`: `timeout = "600s"`

### "FFI code not covered"
- Expected - Rust coverage tools don't track C++ code
- Focus on Rust wrapper coverage
- C++ has separate coverage tracking

### "CI fails but local passes"
- Ensure you're using same flags: `--all-features --workspace`
- Check CI logs for specific failure
- Download coverage artifacts from CI

## Resources

- Full guide: `COVERAGE.md`
- Tarpaulin docs: https://github.com/xd009642/tarpaulin
- LLVM-COV docs: https://github.com/taiki-e/cargo-llvm-cov
- Codecov dashboard: https://codecov.io/gh/CroatiaControlLtd/asterix

## Coverage Checklist

Before submitting a PR:
- [ ] Coverage ≥ 80% overall
- [ ] New code has tests
- [ ] HTML report reviewed
- [ ] No obvious gaps in coverage
- [ ] CI checks pass

---

**Last updated**: 2025-11-01
**Maintained by**: asterix-rs team
