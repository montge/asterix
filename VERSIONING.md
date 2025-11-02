# ASTERIX Version Management

This document describes the version management system for the ASTERIX project.

## Overview

The ASTERIX project maintains multiple components, each with their own versioning:
- **C++ Library**: Main project version (currently 2.8.10)
- **Python Package**: Uses 0.x.x format (currently 0.7.10, maps from C++ 2.8.10)
- **Rust Crate**: Uses semver format (currently 0.1.0)

## Single Source of Truth

The `VERSION` file in the project root is the **single source of truth** for all component versions.

### VERSION File Format

The VERSION file uses a simple key=value format:

```
# ASTERIX Project Version File
# This file is the single source of truth for all component versions
# Format: KEY=value (whitespace around = is ignored)

# Main project version (C/C++)
CPP_VERSION=2.8.10

# Python package version (0.x.x format, maps from C++ version)
PYTHON_VERSION=0.7.10

# Rust crate version (semver format)
RUST_VERSION=0.1.0

# Project-wide version (used for releases, documentation, etc.)
PROJECT_VERSION=2.8.10
```

## Component Version Mappings

### C++ Version (CPP_VERSION)
- Used in: CMakeLists.txt, version.h, Doxyfile
- Format: MAJOR.MINOR.PATCH (e.g., 2.8.10)
- This is the main project version

### Python Version (PYTHON_VERSION)
- Used in: setup.py, asterix/version.py
- Format: 0.MINOR.PATCH (e.g., 0.7.10)
- Mapping: 2.x.y → 0.(x-1).y
- Example: 2.8.10 → 0.7.10

### Rust Version (RUST_VERSION)
- Used in: asterix-rs/Cargo.toml
- Format: Semver (e.g., 0.1.0)
- Independent versioning for Rust crate

## Updating Versions

### Automatic Synchronization

Run the sync script to update all component files from VERSION:

```bash
./scripts/sync-versions.sh
```

This script:
- Updates `asterix-rs/Cargo.toml` with RUST_VERSION
- Updates `asterix/version.py` with PYTHON_VERSION
- Note: CMake reads directly from VERSION file, no sync needed

### Manual Updates

To update a version:

1. Edit `VERSION` file with new version numbers
2. Run `./scripts/sync-versions.sh` to propagate changes
3. Commit changes

Example:
```bash
# Edit VERSION file
vim VERSION

# Sync to all components
./scripts/sync-versions.sh

# Commit
git add VERSION asterix-rs/Cargo.toml asterix/version.py
git commit -m "Bump version to 2.8.11"
```

## How Components Read Versions

### CMake (CMakeLists.txt)
- Reads `CPP_VERSION` or `PROJECT_VERSION` from VERSION file
- Falls back to legacy single-line format
- Generates `version.h` from template using PROJECT_VERSION

### Python (setup.py)
- Reads `PYTHON_VERSION` from VERSION file (preferred)
- Falls back to computing from `CPP_VERSION` using mapping formula
- Falls back to legacy format

### Rust (Cargo.toml)
- Updated via `sync-versions.sh` script
- Cargo uses version from Cargo.toml directly

### GitHub Actions Workflows
- Extract `CPP_VERSION` or `PROJECT_VERSION` from VERSION file
- Used in build-packages.yml, nightly-builds.yml, doxygen.yml

## Version History Mapping

The Python version mapping follows the historical pattern:
- 2.8.0 → 0.7.0
- 2.8.10 → 0.7.10

Formula: `2.x.y → 0.(x-1).y`

## Best Practices

1. **Always update VERSION file first** - It's the single source of truth
2. **Run sync script** - Ensures all components are updated
3. **Test locally** - Verify version propagation before committing
4. **Commit all changes together** - VERSION + all synced files

## Legacy Support

The system maintains backward compatibility:
- Supports old single-line format (just version number)
- Falls back gracefully if keys are missing
- Works with both structured (key=value) and simple formats

