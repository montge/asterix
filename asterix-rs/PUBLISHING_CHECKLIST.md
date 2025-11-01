# Publishing Checklist for asterix_decoder Rust Crate

This checklist ensures the crate is ready for publication on crates.io.

## Pre-Publication Checks

### 1. Code Quality ✅

- [ ] **All tests passing**
  ```bash
  cargo test --all-features
  ```
  - [ ] Unit tests (src/)
  - [ ] Integration tests (tests/)
  - [ ] Doc tests
  - [ ] Benchmarks compile

- [ ] **Clippy warnings resolved**
  ```bash
  cargo clippy --all-features -- -D warnings
  ```
  - [ ] No warnings on stable
  - [ ] No warnings on nightly (best effort)

- [ ] **Code formatted with rustfmt**
  ```bash
  cargo fmt -- --check
  ```

- [ ] **90%+ test coverage achieved**
  ```bash
  cargo tarpaulin --all-features --out Html
  # Open tarpaulin-report.html and verify coverage
  ```

### 2. Documentation ✅

- [ ] **All public APIs documented**
  ```bash
  cargo doc --all-features --no-deps
  # Check that all items have doc comments
  ```

- [ ] **Documentation builds without warnings**
  ```bash
  RUSTDOCFLAGS="--deny warnings" cargo doc --all-features --no-deps
  ```

- [ ] **README.md complete**
  - [ ] Installation instructions
  - [ ] Quick start example
  - [ ] Basic usage examples
  - [ ] Advanced examples
  - [ ] API overview
  - [ ] Performance benchmarks
  - [ ] Platform support matrix
  - [ ] License information
  - [ ] Links to resources

- [ ] **CHANGELOG.md updated**
  - [ ] Version number correct
  - [ ] All changes documented
  - [ ] Breaking changes highlighted
  - [ ] Migration guide (if needed)

- [ ] **Examples compile and run**
  ```bash
  cargo build --examples --all-features
  cargo run --example parse_pcap -- ../install/sample_data/cat048.raw
  ```

### 3. Cargo.toml Metadata ✅

- [ ] **Package metadata complete**
  ```toml
  [package]
  name = "asterix_decoder"
  version = "0.1.0"
  edition = "2021"
  rust-version = "1.70"  # MSRV
  authors = ["Your Name <email@example.com>"]
  license = "GPL-3.0-or-later"
  description = "Rust bindings for ASTERIX ATM surveillance protocol decoder"
  repository = "https://github.com/your-org/asterix"
  documentation = "https://docs.rs/asterix_decoder"
  homepage = "https://github.com/your-org/asterix"
  readme = "README.md"
  keywords = ["asterix", "aviation", "atm", "surveillance", "parser"]
  categories = ["parser-implementations", "aerospace"]
  ```

- [ ] **License file referenced**
  - [ ] LICENSE or LICENSE.txt file exists in repository root
  - [ ] License matches `license` field in Cargo.toml

- [ ] **Repository URL correct**
  - [ ] Points to public repository
  - [ ] Repository is accessible

- [ ] **Keywords appropriate** (max 5)
  - Specific to domain
  - Searchable terms

- [ ] **Categories appropriate** (max 5)
  - From [crates.io categories](https://crates.io/category_slugs)

- [ ] **Readme path correct**
  ```toml
  readme = "README.md"
  ```

### 4. Dependencies ✅

- [ ] **All dependencies have version constraints**
  ```bash
  cargo tree --duplicate
  # Should show no duplicates if possible
  ```

- [ ] **No path dependencies in published crate**
  - Replace `path = "../foo"` with version numbers

- [ ] **Optional dependencies marked correctly**
  ```toml
  [dependencies]
  serde = { version = "1.0", optional = true }
  ```

- [ ] **Build dependencies minimal**
  - Only include what's necessary for build.rs

- [ ] **No dev-only dependencies in [dependencies]**
  - Move test-only deps to [dev-dependencies]

### 5. Security ✅

- [ ] **No security vulnerabilities**
  ```bash
  cargo audit
  ```

- [ ] **No unsafe code in public API**
  - All unsafe confined to ffi module
  - Safety documented

- [ ] **Input validation on all public APIs**
  - Bounds checking
  - Null pointer checks
  - Buffer overflow protection

- [ ] **Memory safety verified**
  ```bash
  # Debug build with sanitizers
  RUSTFLAGS="-Z sanitizer=address" cargo +nightly test
  RUSTFLAGS="-Z sanitizer=leak" cargo +nightly test
  ```

- [ ] **Valgrind clean (Linux)**
  ```bash
  cargo test --no-run
  valgrind --leak-check=full ./target/debug/deps/integration_test-*
  ```

### 6. Performance ✅

- [ ] **Benchmarks run successfully**
  ```bash
  cargo bench --all-features
  ```

- [ ] **Performance meets targets**
  - Rust achieves 95%+ of C++ performance
  - 2-3x faster than Python
  - No performance regressions vs previous version

- [ ] **Memory usage acceptable**
  - No memory leaks
  - Reasonable allocation patterns

### 7. Cross-Platform Compatibility ✅

- [ ] **Builds on all tier-1 platforms**
  - [ ] Linux x86_64
  - [ ] Linux aarch64
  - [ ] macOS x86_64
  - [ ] macOS aarch64 (Apple Silicon)
  - [ ] Windows x86_64

- [ ] **Tests pass on all platforms**
  ```bash
  # CI should verify this automatically
  ```

- [ ] **MSRV (Minimum Supported Rust Version) verified**
  ```bash
  cargo +1.70 check --all-features
  ```

### 8. Continuous Integration ✅

- [ ] **CI pipeline passing**
  - [ ] All platforms
  - [ ] All Rust versions (stable, nightly)
  - [ ] All features enabled/disabled

- [ ] **Coverage uploaded**
  - [ ] Codecov integration working
  - [ ] Coverage meets minimum threshold (90%)

- [ ] **Security audit passing**
  - [ ] cargo-audit clean
  - [ ] No known vulnerabilities

### 9. docs.rs Configuration ✅

- [ ] **Metadata for docs.rs added**
  ```toml
  [package.metadata.docs.rs]
  all-features = true
  rustdoc-args = ["--cfg", "docsrs"]
  targets = ["x86_64-unknown-linux-gnu"]
  ```

- [ ] **Documentation builds on docs.rs**
  - Test locally with docs.rs Docker image if possible

### 10. Version Management ✅

- [ ] **Version number follows semver**
  - 0.1.0 for initial release
  - MAJOR.MINOR.PATCH format

- [ ] **Version updated in all locations**
  - [ ] Cargo.toml
  - [ ] CHANGELOG.md
  - [ ] README.md (if version mentioned)

- [ ] **Git tag created**
  ```bash
  git tag -a v0.1.0 -m "Release version 0.1.0"
  git push origin v0.1.0
  ```

### 11. Legal ✅

- [ ] **License file present**
  - GPL-3.0-or-later license text

- [ ] **Copyright notices correct**
  - All source files have appropriate headers

- [ ] **Third-party licenses acknowledged**
  - Dependencies' licenses compatible with GPL-3.0

- [ ] **No proprietary code included**

### 12. Release Artifacts ✅

- [ ] **CHANGELOG.md entry for this version**
  ```markdown
  ## [0.1.0] - 2025-11-01

  ### Added
  - Initial release of Rust bindings
  - Core parsing functionality
  - Support for all ASTERIX categories
  - Incremental parsing API
  - Serde integration (optional)
  - Comprehensive test suite
  - Performance benchmarks
  ```

- [ ] **Migration guide (if needed)**
  - For users migrating from Python

- [ ] **Examples cover common use cases**
  - At least 3-5 examples in examples/ directory

## Publication Steps

### Step 1: Final Local Verification

```bash
# Clean build from scratch
cargo clean

# Build with all features
cargo build --all-features --release

# Run full test suite
cargo test --all-features

# Run clippy
cargo clippy --all-features -- -D warnings

# Check formatting
cargo fmt -- --check

# Build documentation
cargo doc --all-features --no-deps

# Run benchmarks
cargo bench --all-features

# Security audit
cargo audit

# Check package contents
cargo package --list
```

### Step 2: Dry Run

```bash
# Create package without uploading
cargo package

# Check package can be built from tarball
cd target/package
tar xzf asterix_decoder-0.1.0.crate
cd asterix_decoder-0.1.0
cargo build --all-features
cargo test --all-features
cd ../../../
```

### Step 3: Test Installation

```bash
# Install from local package
cargo install --path .

# Verify installation
asterix_decoder --version
```

### Step 4: Publish to crates.io

```bash
# Login (one time only)
cargo login <your-api-token>

# Publish (DRY RUN FIRST!)
cargo publish --dry-run

# If dry run succeeds, publish for real
cargo publish

# Verify publication
cargo search asterix_decoder
```

### Step 5: Post-Publication

- [ ] **Verify crate appears on crates.io**
  - Visit https://crates.io/crates/asterix_decoder

- [ ] **Check documentation on docs.rs**
  - Visit https://docs.rs/asterix_decoder

- [ ] **Test installation from crates.io**
  ```bash
  cargo install asterix_decoder
  ```

- [ ] **Create GitHub release**
  - Tag: v0.1.0
  - Title: asterix_decoder v0.1.0
  - Description: From CHANGELOG.md
  - Attach binaries (optional)

- [ ] **Announce release**
  - Update main project README
  - Post on relevant forums/communities
  - Social media (if applicable)

- [ ] **Update version for next development cycle**
  ```bash
  # In Cargo.toml, change version to 0.2.0-dev
  git commit -am "Bump version to 0.2.0-dev"
  git push
  ```

## Common Issues and Solutions

### Issue: Documentation doesn't build on docs.rs

**Solution:** Check `[package.metadata.docs.rs]` configuration. Ensure all feature dependencies are available.

### Issue: Tests fail on specific platform

**Solution:** Add platform-specific code paths or skip tests with `#[cfg(target_os = "...")]`.

### Issue: Package size too large (>10MB)

**Solution:** Add items to `.cargo-ignore` file:
```
# .cargo-ignore
target/
*.swp
*.bak
.git/
```

### Issue: Dependency version conflicts

**Solution:** Run `cargo tree --duplicate` and update dependencies to compatible versions.

### Issue: cargo-audit reports vulnerabilities

**Solution:** Update vulnerable dependencies or file exception if no fix available.

## Emergency Rollback

If you need to yank a published version:

```bash
# Yank version (does not delete, just warns users)
cargo yank --version 0.1.0

# Undo yank if needed
cargo yank --version 0.1.0 --undo
```

**Note:** Only yank versions with critical bugs or security issues.

## Maintenance After Publication

- [ ] Monitor issue tracker for bug reports
- [ ] Respond to community feedback
- [ ] Plan next release cycle
- [ ] Update documentation as needed
- [ ] Keep dependencies up to date

---

## Version History

| Version | Date | Status |
|---------|------|--------|
| 0.1.0 | 2025-11-01 | ⏳ Pending Publication |

---

**Last Updated:** 2025-11-01
**Next Review:** Before each publication
