# ASTERIX Documentation

**Official Documentation Site:** https://montge.github.io/asterix/

This repository contains multi-language documentation for the ASTERIX decoder/parser project.

---

## Quick Links

### 📚 Complete Documentation
Visit **https://montge.github.io/asterix/** for the complete documentation site with:
- Landing page with overview
- Build & installation guides
- API references for all languages
- Architecture and design docs

### 🔧 API Documentation by Language

| Language | Documentation | Package | Status |
|----------|--------------|---------|--------|
| **C++** | [C++ API Docs](https://montge.github.io/asterix/cpp/html/) | N/A (executable) | ✅ Active |
| **Python** | [ReadTheDocs](https://asterix.readthedocs.io/) | [PyPI](https://pypi.org/project/asterix-decoder/) | 🔄 Setup pending |
| **Rust** | [docs.rs](https://docs.rs/asterix-decoder) | [crates.io](https://crates.io/crates/asterix-decoder) | 🔄 Not yet published |

### 📖 Key Documentation Files

- **[Build Guide](https://montge.github.io/asterix/BUILD.html)** - Multi-platform build instructions
- **[README.md](README.md)** - Project overview and quick start
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - How to contribute
- **[SECURITY.md](SECURITY.md)** - Security policy
- **[CLAUDE.md](CLAUDE.md)** - Technical architecture guide

---

## Documentation Structure

### GitHub Pages (Main Hub)
```
https://montge.github.io/asterix/
├── /                    - Landing page
├── /BUILD.html          - Build & installation guide
├── /cpp/html/           - C++ API reference (Doxygen)
└── [Future pages]       - Additional guides
```

### ReadTheDocs (Python)
```
https://asterix.readthedocs.io/
├── /                    - Python user guide
├── /api/                - Python API reference
└── /examples/           - Python examples
```

### docs.rs (Rust)
```
https://docs.rs/asterix-decoder/
└── /                    - Rust API reference (auto-generated from crates.io)
```

---

## Building Documentation Locally

### GitHub Pages Site
```bash
# Install Jekyll (one-time setup)
gem install bundler jekyll

# Serve locally
cd /path/to/asterix
jekyll serve --source docs --baseurl ""

# View at http://localhost:4000
```

### C++ API Docs (Doxygen)
```bash
# Install Doxygen
sudo apt-get install doxygen graphviz  # Ubuntu/Debian
brew install doxygen graphviz          # macOS

# Generate
doxygen Doxyfile

# View
open docs/cpp/html/index.html
```

### Python Docs (Sphinx)
```bash
# Install Sphinx
pip install -r requirements-docs.txt

# Build
cd asterix/docs
make html

# View
open _build/html/index.html
```

### Rust Docs (cargo doc)
```bash
# Generate
cd asterix-rs
cargo doc --open

# View
# Opens automatically in browser
```

---

## Contributing to Documentation

### GitHub Pages Content
- Edit markdown files in `docs/` directory
- Push to master branch
- GitHub Actions automatically rebuilds and deploys

### C++ API Docs
- Add Doxygen comments to C++ source code
- Follow Doxygen comment format (see [docs/cpp/README.md](docs/cpp/README.md))
- Push changes - auto-rebuilds on source changes

### Python Docs
- Edit `.rst` files in `asterix/docs/`
- Update docstrings in Python source
- Test locally before committing

### Rust Docs
- Add doc comments (`///` or `//!`) to Rust source
- Use `cargo doc` to test locally
- Published automatically when crate is published to crates.io

---

## Documentation Automation

### GitHub Actions Workflows

**Build Documentation Site** (`.github/workflows/doxygen.yml`):
- Triggers: Changes to C++ source, Doxyfile, or docs/*.md
- Actions: Generate Doxygen docs, deploy to GitHub Pages
- Deploys to: https://montge.github.io/asterix/

**ReadTheDocs** (when activated):
- Triggers: Changes to Python source or `asterix/docs/`
- Actions: Build Sphinx docs
- Deploys to: https://asterix.readthedocs.io/

**docs.rs** (automatic when published):
- Triggers: `cargo publish` to crates.io
- Actions: Build rustdoc documentation
- Deploys to: https://docs.rs/asterix-decoder

---

## Documentation Standards

### General Guidelines
- Use clear, concise language
- Provide working code examples
- Include both quick start and detailed explanations
- Cross-reference related documentation
- Keep examples up-to-date with code

### Language-Specific

**C++ (Doxygen)**:
```cpp
/**
 * @brief Brief description
 * @param name Parameter description
 * @return Return value description
 * @see RelatedClass
 */
```

**Python (Sphinx/reStructuredText)**:
```python
def function(param):
    """Brief description.

    Detailed description.

    Args:
        param (type): Parameter description

    Returns:
        type: Return value description
    """
```

**Rust (rustdoc)**:
```rust
/// Brief description
///
/// Detailed description
///
/// # Examples
/// ```
/// let result = function();
/// ```
pub fn function() -> Result<(), Error> {
```

---

## Support

For documentation questions or issues:
- **General:** [GitHub Discussions](https://github.com/montge/asterix/discussions)
- **Bugs:** [GitHub Issues](https://github.com/montge/asterix/issues)
- **Security:** See [SECURITY.md](SECURITY.md)

---

<small>
Last updated: 2025-11-04 | [Edit this page](https://github.com/montge/asterix/edit/master/DOCUMENTATION.md)
</small>
