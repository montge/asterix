# GitHub Actions Workflows

This directory contains GitHub Actions CI/CD workflows for the ASTERIX decoder project. All workflows follow best practices for safety-critical aviation software development.

## Workflow Overview

### Core Workflows (Existing)

These workflows were in place before radar integration:

| Workflow | Trigger | Purpose | Status Badge |
|----------|---------|---------|--------------|
| **ci-verification.yml** | Push, PR | DO-278 verification, C++ builds, Python tests, coverage | [![DO-278 Verification](https://github.com/montge/asterix/actions/workflows/ci-verification.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/ci-verification.yml) |
| **cross-platform-builds.yml** | Push, PR, Nightly | Windows/macOS/Linux builds, package creation | [![Cross-Platform Builds](https://github.com/montge/asterix/actions/workflows/cross-platform-builds.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/cross-platform-builds.yml) |
| **rust-ci.yml** | Push, PR, Nightly | Rust crate tests, clippy, fmt, benchmarks | [![Rust CI](https://github.com/montge/asterix/actions/workflows/rust-ci.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/rust-ci.yml) |
| **node-ci.yml** | Push, PR | Node.js bindings tests (if applicable) | [![Node CI](https://github.com/montge/asterix/actions/workflows/node-ci.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/node-ci.yml) |
| **nightly-builds.yml** | Nightly (2 AM UTC) | Extended testing, all platforms | [![Nightly Builds](https://github.com/montge/asterix/actions/workflows/nightly-builds.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/nightly-builds.yml) |
| **nightly-security.yml** | Nightly (3 AM UTC) | Security scans, CodeQL analysis | [![Nightly Security](https://github.com/montge/asterix/actions/workflows/nightly-security.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/nightly-security.yml) |
| **weekly-audit.yml** | Weekly (Monday 3 AM UTC) | Comprehensive audit, dependency checks | [![Weekly Audit](https://github.com/montge/asterix/actions/workflows/weekly-audit.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/weekly-audit.yml) |
| **doxygen.yml** | Push (docs/**) | C++ documentation generation | [![Doxygen](https://github.com/montge/asterix/actions/workflows/doxygen.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/doxygen.yml) |
| **build-packages.yml** | Push, PR | Package generation (DEB, RPM, ZIP) | [![Build Packages](https://github.com/montge/asterix/actions/workflows/build-packages.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/build-packages.yml) |
| **sbom-generation.yml** | Push, Release | Software Bill of Materials | [![SBOM Generation](https://github.com/montge/asterix/actions/workflows/sbom-generation.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/sbom-generation.yml) |
| **python-publish.yml** | Release tags | Publish to PyPI | [![PyPI Publish](https://github.com/montge/asterix/actions/workflows/python-publish.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/python-publish.yml) |

### Radar Integration Workflows (New)

These workflows were added for the radar integration feature:

| Workflow | Trigger | Purpose | Status Badge |
|----------|---------|---------|--------------|
| **radar-integration-tests.yml** | Push, PR (radar_integration/**) | Unit tests, example validation, encoder smoke tests | [![Radar Integration Tests](https://github.com/montge/asterix/actions/workflows/radar-integration-tests.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/radar-integration-tests.yml) |
| **encoder-validation.yml** | Push, PR (encoder/**) | Round-trip encoding, binary format validation, regression tests | [![Encoder Validation](https://github.com/montge/asterix/actions/workflows/encoder-validation.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/encoder-validation.yml) |
| **docs-build.yml** | Push, PR (docs/**, examples/**) | Documentation build, example validation, link checking | [![Documentation Build](https://github.com/montge/asterix/actions/workflows/docs-build.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/docs-build.yml) |
| **performance-benchmarks.yml** | Release tags, Manual trigger | Encoding performance, visualization benchmarks | [![Performance Benchmarks](https://github.com/montge/asterix/actions/workflows/performance-benchmarks.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/performance-benchmarks.yml) |
| **security-scan.yml** | Push, PR, Weekly | Bandit, Safety, pip-audit, code quality (Python) | [![Security Scan](https://github.com/montge/asterix/actions/workflows/security-scan.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/security-scan.yml) |

## Workflow Details

### radar-integration-tests.yml

**Comprehensive testing for radar integration Python module**

- **Test Matrix:**
  - Python versions: 3.10, 3.11, 3.12, 3.13
  - Platforms: Ubuntu, macOS, Windows
  - Total: 12 test jobs (3 platforms × 4 Python versions)

- **Test Coverage:**
  - Unit tests: `asterix/radar_integration/test/`
  - Example validation: All 5 examples in `examples/radar_integration/`
  - Encoder smoke tests: Quick functionality verification
  - Coverage reporting to Codecov (target: >80%)

- **Features:**
  - Pip caching for faster builds
  - Headless matplotlib testing (MPLBACKEND=Agg)
  - Visualization tests only on Ubuntu 3.12 to save CI time
  - Timeout protection (2 minutes per example)

- **Triggers:**
  - Push to `master`, `develop`, `feature/**`, `radar/**` branches
  - Pull requests to `master`, `develop`
  - Manual workflow dispatch

### encoder-validation.yml

**Binary format correctness and round-trip encoding verification**

- **Test Jobs:**
  1. **Round-trip tests:** Encode → Decode → Verify
  2. **Binary format validation:** Check ASTERIX structure (CAT, length, FSPEC)
  3. **Regression tests:** Compare against golden reference files (TODO)
  4. **Real sample validation:** Compatibility with existing ASTERIX samples
  5. **Edge case tests:** Min/max values, invalid inputs

- **Validation Criteria:**
  - Category byte = 48 for CAT048
  - Length field matches actual data size
  - FSPEC correctly represents present items
  - All required fields encoded in correct format

- **Triggers:**
  - Push/PR when encoder files change
  - Manual workflow dispatch

### docs-build.yml

**Documentation quality and example code validation**

- **Validation Steps:**
  1. **Example code validation:** All Python files compile successfully
  2. **Documentation build:** Sphinx/MkDocs (if configured)
  3. **Link checking:** Broken links, malformed paths
  4. **Inline code validation:** Extract and test code blocks from README.md
  5. **GitHub Pages deployment:** Auto-deploy on master branch

- **Features:**
  - Detects Sphinx (docs/conf.py) or MkDocs (mkdocs.yml) automatically
  - Validates radar integration README separately
  - Checks for hardcoded paths in examples
  - Deploys to gh-pages branch on master push

- **Triggers:**
  - Push/PR to docs, README, examples
  - Manual workflow dispatch

### performance-benchmarks.yml

**Performance measurement and regression detection**

- **Benchmark Categories:**
  1. **Encoder performance:**
     - CAT048 encoding throughput (plots/second)
     - Bulk encoding (1000 plots)
     - Time per plot (milliseconds)

  2. **Visualization performance:**
     - Plot generation speed
     - Rendering time for different data sizes (100-5000 points)

  3. **Mock radar performance:**
     - Aircraft state update throughput
     - Full radar simulation (speedup vs. realtime)

- **Metrics Collected:**
  - Plots/second
  - MB/second (data rate)
  - Time per operation (ms)
  - Speedup vs. realtime

- **Artifact Retention:** 90 days for historical comparison

- **Triggers:**
  - Release tags (v*)
  - Manual workflow dispatch with optional baseline comparison

### security-scan.yml

**Python code security and vulnerability scanning**

- **Scanner Tools:**
  1. **Bandit:** Python security linter (SAST)
     - Scans `asterix/` and `examples/` directories
     - Severity: Medium+ (configurable)
     - SARIF output for GitHub Security tab

  2. **Safety:** Known vulnerability database
     - Checks installed dependencies
     - JSON output for parsing

  3. **pip-audit:** Alternative dependency scanner
     - Uses PyPI vulnerability database

  4. **Code quality:**
     - Pylint: Code quality and style
     - Flake8: PEP8 compliance
     - Mypy: Type checking

- **Reporting:**
  - SARIF upload to GitHub Security tab
  - JSON artifacts (30-day retention)
  - Summary in workflow logs

- **Triggers:**
  - Push/PR to Python files
  - Weekly (Monday 3 AM UTC)
  - Manual workflow dispatch

## Integration with Existing Workflows

### ci-verification.yml (Updated)

Added radar integration testing to the existing DO-278 verification workflow:

```yaml
- name: Run radar integration tests
  run: |
    # Runs pytest on asterix/radar_integration/test/
    # Coverage reporting to Codecov
    # Skips gracefully if tests not found
```

**Changes:**
- Added `numpy scipy` to pip dependencies
- New test step after unittest
- Coverage flag: `radar-integration`

## Coverage Configuration

### Codecov Integration

All radar integration workflows upload coverage to Codecov:

- **Flags:**
  - `radar-integration` - Radar integration module
  - `python-X.Y` - Python version (3.10, 3.11, 3.12, 3.13)

- **Coverage Targets:**
  - Overall: 90% (DO-278 requirement)
  - Radar integration: 80% (new code)

- **Reports:**
  - XML format (for Codecov)
  - HTML format (artifact upload)
  - Terminal output (workflow logs)

### Coverage Files

- **Configuration:** `asterix-rs/codecov.yml` (Rust), root-level if needed
- **Ignore patterns:** `__pycache__/`, `.git/`, `build/`

## Workflow Best Practices

### 1. Fail-Fast Strategy

Most workflows use `fail-fast: false` in test matrices to:
- See all test failures (not just first)
- Complete platform-specific tests even if others fail
- Provide comprehensive CI feedback

### 2. Dependency Caching

All Python workflows use pip caching:

```yaml
- uses: actions/setup-python@v6
  with:
    python-version: '3.12'
    cache: 'pip'  # Automatic pip cache
```

### 3. Timeout Protection

Example tests use `timeout-minutes: 2` to prevent hangs.

### 4. Artifact Management

- **Short-term (7-14 days):** Test reports, coverage HTML
- **Long-term (30 days):** Security reports, code quality
- **Extended (90 days):** Performance benchmarks

### 5. Cross-Platform Compatibility

Windows-specific handling:
```powershell
# PowerShell for Windows
$exe = "install/bin/asterix.exe"
& $exe --help
```

Unix-specific handling:
```bash
# Bash for Linux/macOS
export LD_LIBRARY_PATH=install/lib
./install/bin/asterix --help
```

## Adding New Workflows

When adding new workflows, follow these guidelines:

1. **Naming:** Use descriptive kebab-case names (e.g., `radar-integration-tests.yml`)

2. **Triggers:** Be specific about paths to avoid unnecessary runs
   ```yaml
   on:
     push:
       paths:
         - 'asterix/radar_integration/**'
         - '.github/workflows/radar-integration-tests.yml'
   ```

3. **Permissions:** Use minimal required permissions
   ```yaml
   permissions:
     contents: read  # Most workflows
     security-events: write  # Only for security scans
   ```

4. **Status Badges:** Add to this README.md and main README.md

5. **Documentation:** Document in this README.md with:
   - Purpose and scope
   - Trigger conditions
   - Key features
   - Artifacts produced

## Troubleshooting

### Common Issues

1. **"No module named 'asterix.radar_integration'"**
   - Ensure `pip install .` runs before tests
   - Check that radar_integration module is in PYTHONPATH

2. **"No radar integration test files found"**
   - Expected during development if tests not yet created
   - Workflows skip gracefully with `::notice::` annotation

3. **Coverage upload failures**
   - `continue-on-error: true` prevents blocking builds
   - Check Codecov token if persistent failures

4. **Timeout on examples**
   - Increase `timeout-minutes` if needed
   - Use headless matplotlib: `MPLBACKEND=Agg`

### Debugging Workflows

1. **Enable debug logging:**
   - Repository Settings → Secrets → Add `ACTIONS_STEP_DEBUG=true`

2. **Re-run with debug:**
   - Workflow run → "Re-run jobs" → "Enable debug logging"

3. **Check artifacts:**
   - Download test reports, coverage HTML from workflow run

4. **Local testing:**
   ```bash
   # Install act (GitHub Actions local runner)
   # https://github.com/nektos/act
   act -j test-radar-integration
   ```

## Maintenance

### Weekly Tasks

- Review failed workflows (especially nightly/weekly)
- Check security scan results
- Update dependencies (Dependabot PRs)

### Monthly Tasks

- Review artifact storage (clean up old artifacts)
- Update workflow documentation
- Check for new GitHub Actions versions

### Quarterly Tasks

- Audit all workflows for efficiency
- Review coverage trends
- Update baseline benchmarks

## References

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [DO-278 Software Considerations (Aviation Safety)](https://en.wikipedia.org/wiki/DO-178C)
- [Codecov Documentation](https://docs.codecov.com/docs)
- [Safety Database](https://github.com/pyupio/safety-db)
- [Bandit Security Linter](https://bandit.readthedocs.io/)

## Status Dashboard

View all workflow runs: [Actions Tab](https://github.com/montge/asterix/actions)

**Current Status (as of last update):**
- ✅ Core workflows: Passing
- ✅ Radar integration workflows: New (testing required)
- ⚠️  Coverage target: 92.2% overall (exceeds 90% requirement)
- 📊 Benchmark baseline: To be established on first run

---

**Last Updated:** 2025-11-23
**Maintained by:** ASTERIX Development Team
