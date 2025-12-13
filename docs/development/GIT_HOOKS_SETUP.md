# ASTERIX Git Hooks Setup Guide

Comprehensive guide to setting up and using automated git hooks for the ASTERIX project.

## Overview

The ASTERIX repository includes two automated git hooks to maintain code quality and catch issues before they reach the repository:

- **Pre-commit hook** (~5 seconds): Fast checks for formatting, linting, and security
- **Pre-push hook** (~60 seconds): Comprehensive checks including builds and tests

These hooks are **optional but highly recommended** for all contributors.

## Quick Start

### 1. Automatic Setup (Recommended)

The hooks are already in place in `.git/hooks/`. Just ensure they're executable:

```bash
chmod +x .git/hooks/pre-commit .git/hooks/pre-push
```

That's it! Hooks will now run automatically on your commits and pushes.

### 2. Install Optional Dependencies

For hooks to work fully, install recommended tools:

```bash
# Python tools
pip install black flake8 cpplint pytest pytest-cov coverage mypy

# System tools (Ubuntu/Debian)
sudo apt-get install clang-format clang-tools valgrind cppcheck

# System tools (macOS)
brew install clang-format valgrind cppcheck
```

### 3. Optional: Pre-Commit Framework

For additional framework-based checks:

```bash
pip install pre-commit
pre-commit install
```

## Hooks in Detail

### Pre-Commit Hook

**Location:** `.git/hooks/pre-commit`

**Execution:** Runs automatically before `git commit`

**Time:** ~5 seconds (designed to be fast)

**Checks:**

1. **Trailing Whitespace**
   - Detects and prevents trailing whitespace in files
   - Ensures consistent file endings

2. **Merge Conflict Markers**
   - Prevents committing unresolved merge conflicts
   - Catches `<<<<<<<`, `=======`, `>>>>>>>`

3. **File Size Limits**
   - No files larger than 1 MB
   - Prevents accidentally committing large binaries or media

4. **Secret Detection**
   - Detects hardcoded passwords, API keys, tokens
   - Scans for common security anti-patterns
   - Prevents accidental credential commits

5. **Python Formatting (black)**
   - Ensures Python code follows black formatting
   - Auto-format with: `black asterix/ tests/ --line-length 100`

6. **Python Linting (flake8)**
   - Checks Python code style and correctness
   - Fix issues: `flake8 asterix/ --max-line-length=100`

7. **C++ Formatting (clang-format)**
   - Ensures C/C++ code follows project formatting
   - Auto-format with: `find . -name '*.cpp' -o -name '*.h' | xargs clang-format -i`

8. **C++ Linting (cpplint)**
   - Checks C/C++ code style (warnings only)
   - Informational - doesn't block commits

9. **Debug Code Detection**
   - Prevents committing debug statements
   - Detects: console.log, debugger, print_debug, cout<<DEBUG, etc.

#### Pre-Commit Hook Output Example

```
==========================================
     ASTERIX Pre-Commit Hook
==========================================

[INFO] === Checking for trailing whitespace ===
[OK] No trailing whitespace found

[INFO] === Checking for merge conflict markers ===
[OK] No merge conflict markers found

[INFO] === Checking for large files (limit: 1 MB) ===
[OK] All files are under 1 MB

[INFO] === Checking for hardcoded secrets ===
[OK] No obvious secrets detected

[INFO] === Checking Python formatting ===
[OK] All Python files are properly formatted

[INFO] === Checking Python linting ===
[OK] All Python files pass linting

[INFO] === Checking C/C++ formatting ===
[OK] All C/C++ files are properly formatted

[INFO] === Checking C/C++ linting ===
[OK] C/C++ files pass linting

[INFO] === Checking for debugging code ===
[OK] No obvious debug code found

==========================================
[OK] All pre-commit checks passed!

To bypass these checks (not recommended):
  git commit --no-verify
```

### Pre-Push Hook

**Location:** `.git/hooks/pre-push`

**Execution:** Runs automatically before `git push`

**Time:** ~30-60 seconds (can be longer if builds fail)

**Checks:**

1. **C++ Build Verification (Production)**
   - Runs: `make clean && make` in src/
   - Ensures code compiles without errors
   - Creates production executable

2. **C++ Build Verification (Debug)**
   - Runs: `make debug` in src/
   - Creates debug executable with debugging symbols
   - Verifies debug build compatibility

3. **C++ Unit Tests**
   - Runs Google Test suite
   - All tests must pass

4. **Python Unit Tests**
   - Runs pytest suite: `pytest asterix/test/ -v`
   - All tests must pass

5. **Integration Tests**
   - Runs: `install/test/test.sh`
   - 11 integration tests validating end-to-end functionality
   - Tests various ASTERIX formats (PCAP, GPS, HDLC, etc.)

6. **Memory Leak Detection (Optional)**
   - Runs valgrind if available
   - Checks for memory leaks in C++ code
   - Non-blocking (warns but doesn't prevent push)

7. **Static Analysis - cppcheck**
   - Analyzes C/C++ code for potential issues
   - Reports warnings and errors
   - Non-blocking (informational only)

8. **Static Analysis - mypy**
   - Python type checking (if type hints exist)
   - Reports type issues
   - Non-blocking (informational only)

9. **Coverage Check**
   - Verifies test coverage meets minimum threshold (80%)
   - Warns if coverage below threshold
   - Non-blocking

#### Pre-Push Hook Output Example

```
==========================================
     ASTERIX Pre-Push Hook
==========================================

=== Building C++ (Production) ===
[INFO] Cleaning previous build...
[INFO] Building production binary...
[OK] C++ production build successful

=== Building C++ (Debug) ===
[INFO] Building debug binary...
[OK] C++ debug build successful

=== Running C++ Unit Tests ===
[INFO] Running: /path/to/install/test/test_asterix
[OK] All C++ unit tests passed (1 test suite(s))

=== Running Python Unit Tests ===
[INFO] Running pytest...
[OK] Python tests passed

=== Running Integration Tests ===
[INFO] Running: /path/to/install/test/test.sh
Test filter ... OK
Test json CAT_062_065 ... OK
Test json CAT_001_002 ... OK
[OK] Integration tests passed

=== Checking for memory leaks (valgrind) ===
[OK] No memory leaks detected

=== Running cppcheck for C/C++ static analysis ===
[OK] No critical issues found by cppcheck

=== Running mypy for Python type checking ===
[OK] Python type checking passed

=== Checking test coverage ===
Current coverage:
asterix/__init__.py  85%
asterix/parser.py    92%

[OK] Coverage meets minimum threshold (89%)

==========================================
Pre-Push Check Summary (45s)
==========================================
[OK] All pre-push checks passed!

Ready to push! Your commits are:
abc1234 fix: update parser logic
def5678 feat: add new feature

To bypass these checks (not recommended):
  git push --no-verify
```

## Usage

### Normal Workflow (With Hooks)

```bash
# Make changes to your code
git add .

# Commit - pre-commit hook runs automatically
git commit -m "feat: add new feature"

# Push - pre-push hook runs automatically
git push origin feature/my-feature
```

### Bypassing Hooks (Not Recommended)

Sometimes you might need to bypass hooks temporarily:

```bash
# Bypass pre-commit hook only
git commit --no-verify -m "WIP: work in progress"

# Bypass pre-push hook only
git push --no-verify origin branch

# Bypass both hooks
git commit --no-verify && git push --no-verify
```

**Important:** Bypassing hooks defeats their purpose. Only do this if you have a specific reason and plan to fix issues before the next push.

## Installation Requirements

### Minimal Setup (Hooks Run With Reduced Checks)

```bash
# Hooks will run but skip unavailable tools
chmod +x .git/hooks/pre-commit .git/hooks/pre-push
```

### Recommended Setup (Full Functionality)

**Python Tools:**
```bash
pip install black flake8 cpplint pytest pytest-cov coverage mypy
```

**C++ Tools (Ubuntu/Debian):**
```bash
sudo apt-get install clang-format clang-tools valgrind cppcheck
```

**C++ Tools (macOS):**
```bash
brew install clang-format valgrind cppcheck
```

**C++ Tools (Fedora/RHEL):**
```bash
sudo dnf install clang-tools-extra valgrind cppcheck
```

### Complete Setup (Including Pre-Commit Framework)

```bash
# Install pre-commit framework
pip install pre-commit

# Set up hooks
pre-commit install

# Install all dependencies
pre-commit install --install-hooks
```

## Troubleshooting

### Hooks Not Running

**Problem:** Hooks don't execute on commit or push

**Solutions:**

```bash
# Check if hooks are executable
ls -la .git/hooks/pre-commit .git/hooks/pre-push

# Make executable if needed
chmod +x .git/hooks/pre-commit .git/hooks/pre-push

# Verify hooks exist and are readable
file .git/hooks/pre-commit
```

### Hook Takes Too Long

**Problem:** Pre-commit or pre-push hook takes longer than expected

**For Pre-Commit:**
- Usually indicates missing tools (uses graceful fallback)
- Shouldn't exceed 5 seconds with all tools installed

**For Pre-Push:**
- Expected to take 30-60 seconds if builds/tests pass
- May take longer if builds fail
- Can be run in background: `git push --no-verify && pre-push-hook &`

**Solution:**
```bash
# Install missing tools to speed up checks
pip install black flake8 cpplint pytest pytest-cov

# For C++, install clang-format, valgrind, cppcheck
sudo apt-get install clang-format valgrind cppcheck  # Ubuntu
brew install clang-format valgrind cppcheck  # macOS
```

### Pre-Commit Hook Fails: Formatting Issues

**Problem:** Hook fails because code doesn't match formatting standards

**For Python:**
```bash
# Auto-format Python code
black asterix/ tests/ --line-length 100

# Then try committing again
git add .
git commit -m "style: auto-format code"
```

**For C++:**
```bash
# Auto-format C++ code
find . -name '*.cpp' -o -name '*.h' | xargs clang-format -i

# Then stage and commit
git add .
git commit -m "style: auto-format code"
```

### Pre-Commit Hook Fails: Secrets Detected

**Problem:** Hook detects potential hardcoded secrets

**Solution:**
```bash
# Option 1: Use environment variables instead
export API_KEY="your_key"

# Option 2: Use .env file (add to .gitignore)
echo "API_KEY=your_key" > .env.local

# Option 3: Remove the credential from code
# Edit file to remove the hardcoded value

# Then try committing again
git add .
git commit -m "security: remove hardcoded credentials"
```

### Pre-Push Hook Fails: Build Error

**Problem:** C++ build fails in pre-push hook

**Solutions:**
```bash
# Run build locally to debug
cd src
make clean
make

# Fix the compilation errors

# Try pushing again
git push origin feature/my-feature
```

### Pre-Push Hook Fails: Tests Fail

**Problem:** Unit or integration tests fail in pre-push hook

**Solutions:**
```bash
# Run tests locally
pytest asterix/test/ -v

# Run C++ tests
cd build
ctest --output-on-failure

# Run integration tests
cd install/test
./test.sh

# Fix failing tests and commit
git add .
git commit -m "test: fix failing tests"

# Try pushing again
git push origin feature/my-feature
```

### Hook Permission Denied

**Problem:** `Permission denied: .git/hooks/pre-commit`

**Solution:**
```bash
chmod +x .git/hooks/pre-commit .git/hooks/pre-push
```

### Windows: Hooks Not Executing

**Problem:** Hooks don't run on Windows/WSL

**Solutions:**

1. Ensure git line endings are correct:
```bash
# Global configuration
git config --global core.autocrlf true
git config --global core.safecrlf warn

# For this repository
git config core.autocrlf true
```

2. Verify hooks use Unix line endings:
```bash
# WSL/Git Bash
file .git/hooks/pre-commit
# Should say: POSIX shell script, ASCII text, with CRLF line terminators

# Convert if needed
dos2unix .git/hooks/pre-commit .git/hooks/pre-push
```

### Pre-Commit Framework Issues

**Problem:** Pre-commit framework hooks fail or conflict with shell hooks

**Solution:**
```bash
# Uninstall framework if causing issues
pip uninstall pre-commit

# Remove framework hooks
rm -f .git/hooks/pre-commit .git/hooks/pre-push

# Restore shell-based hooks
git checkout .git/hooks/pre-commit .git/hooks/pre-push
chmod +x .git/hooks/pre-commit .git/hooks/pre-push
```

## Maintenance

### Updating Hooks

Hooks are part of the repository. To get updates:

```bash
# Pull latest changes
git pull origin master

# Ensure hooks are executable
chmod +x .git/hooks/pre-commit .git/hooks/pre-push
```

### Checking Hook Versions

```bash
# View pre-commit hook
head -20 .git/hooks/pre-commit

# View pre-push hook
head -20 .git/hooks/pre-push
```

### Customizing Hooks

While not recommended, you can customize hooks for your local environment:

```bash
# View hook to understand what it does
cat .git/hooks/pre-commit

# Make local changes (these won't be committed)
vim .git/hooks/pre-commit

# To restore original
git checkout .git/hooks/pre-commit
```

## Best Practices

1. **Always run hooks locally before pushing**
   - Hooks catch issues that would block CI

2. **Never commit with --no-verify** unless:
   - You have a specific temporary need
   - You plan to fix issues immediately after

3. **Keep your tools updated**
   ```bash
   pip install --upgrade black flake8 pytest
   brew upgrade clang-format valgrind cppcheck
   ```

4. **Run pre-push checks manually before pushing**
   ```bash
   # Simulate what pre-push hook will do
   cd src && make clean && make && make debug
   pytest asterix/test/ -v
   cd ../install/test && ./test.sh
   ```

5. **Contribute hook improvements**
   - If you find issues with hooks, report them
   - Suggest improvements for better coverage
   - Submit PRs to enhance hook functionality

## Integration with CI/CD

These hooks complement the CI/CD pipeline (GitHub Actions, etc.):

- **Pre-commit hook**: Catches 80% of issues locally before they're staged
- **Pre-push hook**: Ensures code is buildable and tests pass before push
- **CI/CD**: Final comprehensive check including platform-specific builds

All three layers work together to ensure code quality.

## See Also

- [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines
- [.git/hooks/pre-commit](.git/hooks/pre-commit) - Pre-commit hook implementation
- [.git/hooks/pre-push](.git/hooks/pre-push) - Pre-push hook implementation
- [.pre-commit-config.yaml](.pre-commit-config.yaml) - Pre-commit framework configuration
- [CLAUDE.md](CLAUDE.md) - Project overview and architecture

## Questions?

For issues or questions about git hooks:

1. Check this guide's troubleshooting section
2. Review the [CONTRIBUTING.md](CONTRIBUTING.md) Git Hooks section
3. Open an issue on GitHub with details about your problem
4. Check existing issues for similar problems

---

**Last Updated:** 2025-11-01

**Hook Version:** 1.0

**Compatibility:** POSIX-compliant shells (bash, zsh, dash)
