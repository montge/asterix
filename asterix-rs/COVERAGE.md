# Code Coverage Guide for asterix-rs

This document describes the code coverage standards, tools, and processes for the Rust bindings.

## Coverage Targets

Matching the standards set by C++ and Python implementations:

- **Minimum per-module coverage**: 80%
- **Target overall coverage**: 90%
- **CI enforcement**: Builds fail if coverage drops below 80%

## Coverage Tools

### Primary: cargo-tarpaulin

Tarpaulin is the primary coverage tool with LLVM engine support.

**Installation:**
```bash
cargo install cargo-tarpaulin
```

**Usage:**
```bash
cd asterix-rs

# Generate all coverage formats
cargo tarpaulin \
  --verbose \
  --all-features \
  --workspace \
  --timeout 300 \
  --engine llvm \
  --out Xml \
  --out Html \
  --out Json \
  --out Lcov \
  --output-dir coverage \
  --fail-under 80

# View HTML report
open coverage/index.html  # macOS
xdg-open coverage/index.html  # Linux
```

**Configuration:**
Coverage settings are defined in `.tarpaulin.toml`:
- Output formats: HTML, XML (Cobertura), JSON, LCOV
- Excludes test files, benchmarks, examples
- 5-minute timeout per test
- LLVM engine for accurate coverage

### Alternative: cargo-llvm-cov

LLVM-based coverage tool providing similar functionality with different internals.

**Installation:**
```bash
rustup component add llvm-tools-preview
cargo install cargo-llvm-cov
```

**Usage:**
```bash
cd asterix-rs

# Generate LCOV coverage
cargo llvm-cov --all-features --workspace --lcov --output-path coverage-llvm/lcov.info

# Generate HTML report
cargo llvm-cov report --all-features --workspace --html --output-dir coverage-llvm/html

# Generate JSON report
cargo llvm-cov report --all-features --workspace --json --output-path coverage-llvm/coverage.json

# View HTML report
open coverage-llvm/html/index.html
```

## CI/CD Integration

### GitHub Actions Workflow

Coverage is automatically checked on every push and PR via `.github/workflows/rust-ci.yml`:

1. **coverage-tarpaulin job**: Primary coverage with tarpaulin
   - Generates HTML, XML, JSON, LCOV reports
   - Uploads to Codecov with `rust,tarpaulin` flags
   - Fails if coverage < 80%
   - Artifacts retained for 30 days

2. **coverage-llvm-cov job**: Alternative coverage with llvm-cov
   - Cross-validates tarpaulin results
   - Uploads to Codecov with `rust,llvm-cov` flags
   - Provides secondary coverage metric

### Coverage Enforcement

The CI pipeline enforces coverage thresholds:

```bash
# Tarpaulin built-in enforcement
cargo tarpaulin --fail-under 80

# Additional threshold check via Python
COVERAGE=$(python3 -c "import json; data=json.load(open('coverage/tarpaulin-report.json')); print(f'{data.get(\"coverage\", 0):.2f}')")

if (( $(echo "$COVERAGE < 80" | bc -l) )); then
  echo "FAILED: Coverage ${COVERAGE}% is below 80%"
  exit 1
fi
```

## Codecov Integration

### Upload Process

Both coverage tools upload to Codecov:
- **Flags**: `rust`, `tarpaulin` or `llvm-cov`
- **Format**: Cobertura XML (tarpaulin), LCOV (llvm-cov)
- **Token**: Required via `CODECOV_TOKEN` secret

### Badges

Coverage badges are displayed in README.md:
```markdown
[![codecov](https://codecov.io/gh/CroatiaControlLtd/asterix/branch/master/graph/badge.svg?flag=rust)](https://codecov.io/gh/CroatiaControlLtd/asterix)
```

## Local Development

### Running Coverage Locally

```bash
# Quick coverage check (tarpaulin)
cd asterix-rs
cargo tarpaulin --all-features --workspace

# Detailed coverage with HTML (tarpaulin)
cargo tarpaulin --all-features --workspace --out Html --output-dir coverage
open coverage/index.html

# Alternative with llvm-cov
cargo llvm-cov --all-features --workspace --html
open target/llvm-cov/html/index.html
```

### Interpreting Results

**Coverage metrics:**
- **Line coverage**: Percentage of lines executed during tests
- **Function coverage**: Percentage of functions called during tests
- **Branch coverage**: Percentage of decision branches taken (limited in Rust)

**HTML reports show:**
- Overall coverage percentage
- Per-file coverage breakdown
- Per-function coverage
- Highlighted uncovered lines (red)
- Partially covered lines (yellow)
- Fully covered lines (green)

### Improving Coverage

1. **Identify gaps**: Use HTML report to find uncovered code
2. **Write tests**: Add unit/integration tests for uncovered paths
3. **Edge cases**: Test error conditions, boundary values
4. **Integration tests**: Use real ASTERIX data samples
5. **Verify**: Re-run coverage to confirm improvement

## Coverage Exclusions

Certain code is excluded from coverage analysis (via `.tarpaulin.toml`):
- Test files (`tests/`, `**/tests/*`)
- Benchmark files (`benches/`, `**/benches/*`)
- Example files (`examples/`, `**/examples/*`)
- Build scripts (`build.rs`)
- Cargo dependencies (`*/.cargo/*`)

## Comparison with C++/Python

| Metric | C++ | Python | Rust (Target) |
|--------|-----|--------|---------------|
| Coverage Tool | gcov/lcov | pytest-cov | tarpaulin/llvm-cov |
| Current Coverage | 45.5% | 97% | TBD |
| Module Target | 80% | 80% | 80% |
| Overall Target | 90% | 90% | 90% |
| CI Enforcement | ⚠️ Planned | ✅ Yes | ✅ Yes |
| HTML Reports | ✅ Yes | ✅ Yes | ✅ Yes |
| Codecov Upload | ✅ Yes | ✅ Yes | ✅ Yes |

## Best Practices

1. **Run coverage locally** before pushing
2. **Never reduce coverage** without good reason
3. **Document exclusions** if any code can't be tested
4. **Test edge cases** not just happy paths
5. **Review HTML reports** to understand gaps
6. **Keep tests fast** to avoid timeout issues
7. **Use both tools** if results differ significantly

## Troubleshooting

### Timeout Issues
```bash
# Increase timeout in .tarpaulin.toml or command line
cargo tarpaulin --timeout 600
```

### Missing Coverage for FFI Code
- FFI/C++ interop may not be fully covered
- Focus on Rust wrapper logic coverage
- Use integration tests to verify end-to-end behavior

### Different Results Between Tools
- Compare tarpaulin vs llvm-cov results
- Investigate discrepancies in specific files
- Report issues to tool maintainers if reproducible

### CI Failures
```bash
# Check coverage threshold
cat coverage/tarpaulin-report.json | python3 -m json.tool | grep coverage

# Review uncovered code in artifacts
# Download coverage-tarpaulin-reports artifact from CI
# Open coverage/index.html locally
```

## Resources

- [cargo-tarpaulin Documentation](https://github.com/xd009642/tarpaulin)
- [cargo-llvm-cov Documentation](https://github.com/taiki-e/cargo-llvm-cov)
- [Codecov Documentation](https://docs.codecov.com/)
- [Rust Testing Guide](https://doc.rust-lang.org/book/ch11-00-testing.html)

## Maintenance

This coverage configuration should be reviewed and updated:
- When Rust/toolchain versions change
- If coverage tools add new features
- When coverage targets are adjusted
- As the codebase evolves

Last updated: 2025-11-01
