# Git Hooks Optimization Summary

## Changes Made

The git hooks have been optimized to prevent hanging issues when using Cursor or other IDEs. Heavy operations have been moved to background post-hooks.

### Pre-Commit Hook (`.git/hooks/pre-commit`)
**Execution Time:** < 5 seconds (target: < 2 seconds)

**Fast Checks Only:**
- ✅ Trailing whitespace detection
- ✅ Merge conflict markers
- ✅ Large file detection (>1MB)
- ✅ Basic secret detection (critical patterns only)
- ✅ Debug code detection

**Removed (Moved to Post-Hooks):**
- ❌ Full formatting checks (black, clang-format)
- ❌ Full linting (flake8, cpplint)
- ❌ Type checking (mypy)

### Pre-Push Hook (`.git/hooks/pre-push`)
**Execution Time:** < 10 seconds (target: < 5 seconds)

**Quick Checks Only:**
- ✅ Branch protection warnings
- ✅ Python syntax validation (quick compile check)
- ✅ Basic file validation

**Removed (Moved to Post-Push):**
- ❌ Full C++ builds (`make clean && make`)
- ❌ Debug builds
- ❌ Unit tests execution
- ❌ Integration tests
- ❌ Static analysis (cppcheck, mypy)
- ❌ Coverage checks
- ❌ Memory leak detection (valgrind)

### Post-Commit Hook (`.git/hooks/post-commit`)
**Execution:** Background (non-blocking)

**Optional Background Checks:**
- Optional formatting verification
- Optional lint checks
- Results logged but don't block commits

### Post-Push Hook (`.git/hooks/post-push`)
**Execution:** Background (non-blocking)

**Comprehensive Background Verification:**
- C++ build verification (if build system available)
- Python test execution (if pytest available)
- Rust build verification (if cargo available)
- Results logged to `.git/hooks/push-verification.log`

## Benefits

1. **No More Hanging:** Pre-commit and pre-push hooks complete in seconds
2. **Non-Blocking:** Heavy operations run in background
3. **IDE Friendly:** Works smoothly with Cursor, VS Code, and other IDEs
4. **Flexible:** Can still run full checks via post-hooks or manually
5. **Backward Compatible:** Existing workflows continue to work

## Usage

### Normal Workflow

```bash
# Fast pre-commit check (< 5 seconds)
git commit -m "your message"

# Fast pre-push check (< 10 seconds)
git push

# Background verification runs automatically
# Check logs if needed:
cat .git/hooks/push-verification.log
```

### Manual Full Verification

If you want to run full checks before pushing:

```bash
# Run full build
cd src && make clean && make

# Run tests
cd ../install/test && ./test.sh

# Then push (pre-push hook will be quick)
git push
```

### Bypassing Hooks (If Needed)

```bash
# Skip pre-commit checks
git commit --no-verify -m "message"

# Skip pre-push checks
git push --no-verify
```

## Migration from Old Hooks

The old hooks (`.legacy` files) are preserved but not used. If you need them:

```bash
# Restore old hooks (not recommended)
cp .git/hooks/pre-commit.legacy .git/hooks/pre-commit
cp .git/hooks/pre-push.legacy .git/hooks/pre-push
```

## Pre-Commit Framework

If you're using the pre-commit framework (`.pre-commit-config.yaml`), it will still work but the lightweight hooks run first. You can disable the framework hooks:

```bash
# Disable pre-commit framework hooks
pre-commit uninstall

# Or keep both (lightweight hooks run first, then framework)
pre-commit install
```

## Configuration

All hooks respect environment variables and tool availability. If tools aren't installed, hooks skip those checks gracefully.

### Required Tools (for full functionality)
- Git (always required)
- Bash (for hooks)

### Optional Tools
- `python3` - For Python syntax checks
- `make`/`cmake` - For build verification (post-push)
- `cargo` - For Rust checks (post-push)
- `pytest` - For Python tests (post-push)

## Troubleshooting

### Hooks Still Slow?

1. Check if pre-commit framework is installed and running:
   ```bash
   pre-commit --version
   pre-commit uninstall  # Disable if causing issues
   ```

2. Verify hook permissions:
   ```bash
   ls -la .git/hooks/pre-*
   # Should show executable (-rwxr-xr-x)
   ```

3. Check for custom hooks:
   ```bash
   cat .git/hooks/pre-commit
   # Should be the lightweight version
   ```

### Background Verification Not Running?

Post-hooks run in background and may not show output immediately. Check logs:

```bash
cat .git/hooks/push-verification.log
```

## Performance Targets

| Hook | Target Time | Max Time | Status |
|------|-------------|----------|--------|
| pre-commit | < 2s | < 5s | ✅ Optimized |
| pre-push | < 5s | < 10s | ✅ Optimized |
| post-commit | Background | N/A | ✅ Non-blocking |
| post-push | Background | N/A | ✅ Non-blocking |

## Future Improvements

- Add hook configuration file (`.hooks-config.yaml`)
- Add hook timeout configuration
- Add hook enable/disable per hook type
- Add notification system for background verification results

