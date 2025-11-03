# Software Bill of Materials (SBOM)

This directory contains CycloneDX format SBOM files for the ASTERIX project.

## Files

- **asterix-sbom.json** - Main SBOM for C++ core library (version tracked via git tags)
- **asterix-python-sbom.json** - SBOM for Python bindings (if applicable)
- **asterix-rust-sbom.json** - SBOM for Rust crate (if applicable)

## Version Information

- **Current Version**: 2.8.10
- **Git Tag**: v2.8.10
- **Generated**: 2025-11-03T18:53:30Z
- **Commit**: 51d003df2d5d0ec8ca44f062f0e1d1a7bada4277

## How to Use

SBOM files are automatically generated and committed to this repository when the version changes.
Version information is tracked via git tags, not in the filename.

To view version for a specific commit:
```bash
git show <commit>:sbom/asterix-sbom.json | jq .metadata.component.version
```

To view SBOM for a specific version tag:
```bash
git show v2.8.10:sbom/asterix-sbom.json
```
