# Git Hooks Quick Reference

## Installation (Pick One)

### Option 1: One-Command Setup (Recommended)
```bash
bash INSTALL_HOOKS.sh
```

### Option 2: Manual Setup
```bash
chmod +x .git/hooks/pre-commit .git/hooks/pre-push
```

### Option 3: With Pre-Commit Framework
```bash
pip install pre-commit
pre-commit install
```

## Normal Workflow

```bash
# Make changes
git add .
git commit -m "feat: add feature"  # Pre-commit hook runs automatically
git push origin feature/branch      # Pre-push hook runs automatically
```

## Bypassing Hooks (Emergency Only)

```bash
git commit --no-verify              # Skip pre-commit
git push --no-verify                # Skip pre-push
git commit --no-verify && git push --no-verify  # Skip both (not recommended!)
```

## Hook Details

### Pre-Commit Hook (~5 seconds)
Runs automatically before `git commit`

**Checks:**
- Trailing whitespace
- Merge conflict markers
- Large files (>1MB)
- Hardcoded secrets (passwords, API keys)
- Python formatting (black)
- Python linting (flake8)
- C++ formatting (clang-format)
- C++ linting (cpplint)
- Debug code

### Pre-Push Hook (~30-60 seconds)
Runs automatically before `git push`

**Checks:**
- C++ production build
- C++ debug build
- Python tests
- C++ unit tests
- Integration tests
- Memory leaks (optional)
- Static analysis (optional)
- Coverage (optional)

## Common Issues & Fixes

### Hooks not running
```bash
chmod +x .git/hooks/pre-commit .git/hooks/pre-push
```

### Python formatting issue
```bash
black asterix/ tests/ --line-length 100
```

### C++ formatting issue
```bash
find . -name '*.cpp' -o -name '*.h' | xargs clang-format -i
```

### Pre-push fails - build error
```bash
cd src && make clean && make
```

### Pre-push fails - tests fail
```bash
pytest asterix/test/ -v
cd build && ctest --output-on-failure
```

### Install missing tools
```bash
# Python
pip install black flake8 cpplint pytest pytest-cov coverage mypy

# Ubuntu/Debian
sudo apt-get install clang-format valgrind cppcheck

# macOS
brew install clang-format valgrind cppcheck
```

## Hook Files

| File | Purpose | Size |
|------|---------|------|
| `.git/hooks/pre-commit` | Fast checks before commit | 14 KB |
| `.git/hooks/pre-push` | Comprehensive checks before push | 13 KB |
| `.pre-commit-config.yaml` | Pre-commit framework config | 3 KB |
| `GIT_HOOKS_SETUP.md` | Full documentation | 500 lines |
| `CONTRIBUTING.md` | Updated guidelines | +200 lines |
| `INSTALL_HOOKS.sh` | Automated setup script | 6 KB |

## Documentation

- **Setup & Troubleshooting:** [GIT_HOOKS_SETUP.md](GIT_HOOKS_SETUP.md)
- **Implementation Details:** [GIT_HOOKS_IMPLEMENTATION_SUMMARY.md](GIT_HOOKS_IMPLEMENTATION_SUMMARY.md)
- **Development Guidelines:** [CONTRIBUTING.md](CONTRIBUTING.md) - Git Hooks section
- **Contributing Guide:** [CONTRIBUTING.md](CONTRIBUTING.md)

## Time Savings

| Issue | Time Saved |
|-------|-----------|
| Prevent formatting commit | 5 min |
| Prevent secret commit | 30+ min |
| Catch build error locally | 10+ min |
| Prevent test failure push | 15+ min |
| **Per developer per week** | **2+ hours** |

## Support

For issues:
1. Check [GIT_HOOKS_SETUP.md](GIT_HOOKS_SETUP.md) troubleshooting section
2. Review [CONTRIBUTING.md](CONTRIBUTING.md) Git Hooks section
3. Verify tools are installed
4. Open GitHub issue with error details

---

**Quick Links:** [Setup Guide](GIT_HOOKS_SETUP.md) | [Full Documentation](GIT_HOOKS_IMPLEMENTATION_SUMMARY.md) | [Contributing](CONTRIBUTING.md)
