# Next Upstream Sync Checklist

**Last Sync:** 2025-11-01 (v2.8.10)
**Next Sync:** TBD (when CroatiaControlLtd/asterix has new releases)

## ⚠️ IMPORTANT: Clean Git History BEFORE Next Merge

**Current Issue:**
- ~1330 build artifacts (CMakeFiles/, _deps/) remain in git history from October 2025 commits
- They're hidden by .gitignore and don't affect daily work
- BUT they will complicate the next upstream merge if not cleaned first

**Why Clean Before Merge:**
- Git merge tracking relies on commit SHAs
- BFG rewrites all commit SHAs
- If we BFG AFTER merging, next merge won't recognize the merge point
- If we BFG BEFORE merging, next merge works normally

---

## Pre-Sync Checklist (Run BEFORE `git fetch upstream`)

### Step 1: Clean Git History (15 minutes)

**⚠️ WARNING: This requires force-push and will break existing clones!**

```bash
# 1. Backup first!
cd /tmp
git clone --mirror /path/to/asterix asterix-backup-$(date +%Y%m%d).git

# 2. Download BFG Repo-Cleaner
cd /tmp
wget https://repo1.maven.org/maven2/com/madgag/bfg/1.14.0/bfg-1.14.0.jar

# 3. Run BFG to remove build artifacts
cd /path/to/asterix

# Remove CMakeFiles directories
java -jar /tmp/bfg-1.14.0.jar --delete-folders CMakeFiles --no-blob-protection .

# Remove _deps directories
java -jar /tmp/bfg-1.14.0.jar --delete-folders _deps --no-blob-protection .

# Remove .egg-info directories
java -jar /tmp/bfg-1.14.0.jar --delete-folders '*.egg-info' --no-blob-protection .

# 4. Clean up git database
git reflog expire --expire=now --all
git gc --prune=now --aggressive

# 5. Verify cleanup
git ls-tree -r HEAD --name-only | grep -E "CMakeFiles|_deps" | wc -l
# Should show: 0

# 6. Force push (DESTRUCTIVE - notify team first!)
git push --force origin master
```

**Time Required:** ~15 minutes
**Risk:** Medium (breaks existing clones - acceptable for personal fork)
**Benefit:** Clean history, easier future merges

### Step 2: Verify Upstream Remote (1 minute)

```bash
# Check upstream is configured
git remote -v
# Should see: upstream  https://github.com/CroatiaControlLtd/asterix.git

# If not present:
git remote add upstream https://github.com/CroatiaControlLtd/asterix.git
```

### Step 3: Check Upstream for Updates (2 minutes)

```bash
# Fetch latest upstream
git fetch upstream

# See what's new
git log --oneline master..upstream/master | head -20

# Check for conflicts
git merge-tree $(git merge-base master upstream/master) master upstream/master | head -20
```

### Step 4: Review What Changed (5 minutes)

```bash
# See file changes in upstream
git diff --stat master...upstream/master

# See commit messages
git log --oneline --graph master...upstream/master

# Check for new categories
git diff master...upstream/master -- '*/asterix.ini' '*/asterix_cat*.xml'
```

### Step 5: Merge Upstream (5 minutes)

```bash
# If history was cleaned with BFG, normal merge works:
git merge --no-ff upstream/master -m "Merge upstream/master: [describe changes]"

# If history NOT cleaned, use theirs strategy for conflicts:
# git merge upstream/master -X theirs --no-commit
# git status  # review
# git commit
```

### Step 6: Validate Merge (10 minutes)

```bash
# Run all tests
cd src && make clean && make && make install
cd ../install/test && ./test.sh

# Verify security patches still present
grep -r "fstack-protector-strong" src/

# Check coverage tools
python -m pytest --cov=asterix asterix/test/
cd asterix-rs && cargo test --all-features

# Verify version updated
cat src/main/version.h
cat asterix/version.py
```

### Step 7: Update Package Versions (5 minutes)

```bash
# Update version in packaging files
vim debian/changelog  # Update to new version
vim asterix.spec      # Update to new version
vim .github/workflows/nightly-builds.yml  # Update version references
```

### Step 8: Push and Monitor (5 minutes)

```bash
# Push merge
git push origin master

# Monitor CI
gh run watch

# Check new builds
gh run list --limit 5
```

---

## Post-Merge Checklist

- [ ] All tests passing (12/12 integration tests)
- [ ] Security patches verified (grep for security flags)
- [ ] Performance benchmarks run (no regression)
- [ ] Coverage maintained (Python ≥97%, C++ ≥45%, Rust target 90%)
- [ ] Documentation updated (README, CLAUDE.md with new categories)
- [ ] Nightly builds working
- [ ] Cross-platform CI passing (50+ configs)
- [ ] Dependabot PRs merged

---

## Known Risks & Mitigations

| Risk | Probability | Mitigation |
|------|-------------|------------|
| **Merge conflicts** | Medium | Use `-X theirs` or resolve manually |
| **Test failures** | Low | Update tests for new categories |
| **Version mismatches** | Medium | Update all version files |
| **Build failures** | Low | Upstream rarely changes build system |
| **Security regression** | Very Low | Verify compiler flags preserved |

---

## Emergency Rollback

**If merge goes wrong:**

```bash
# Option 1: Abort merge (during merge)
git merge --abort

# Option 2: Revert merge commit (after merge)
git revert -m 1 HEAD
git push origin master

# Option 3: Reset to before merge
git reset --hard HEAD~1
git push --force origin master  # Only if not shared!
```

---

## Success Criteria

Merge is successful when:
- ✅ All 12 integration tests pass
- ✅ No valgrind memory leaks
- ✅ Security compiler flags present (`-fstack-protector-strong`, `-D_FORTIFY_SOURCE=2`)
- ✅ Coverage maintained or improved
- ✅ CI workflows pass on all platforms
- ✅ Version numbers updated correctly
- ✅ New categories (if any) documented

---

## Notes for Next Sync

**Date of This Document:** 2025-11-01
**Current Version:** 2.8.10
**Expected Next Sync:** Mid-2026 (when upstream has new releases)

**Recommendations:**
1. ⚠️ **CLEAN HISTORY FIRST** using BFG (see Step 1 above)
2. Monitor upstream: `git fetch upstream` monthly
3. Subscribe to https://github.com/CroatiaControlLtd/asterix/releases
4. Check for new ASTERIX categories at https://zoranbosnjak.github.io/asterix-specs/

**Our Unique Features to Preserve:**
- C++23/C23 optimizations (deduced this, ranges)
- Rust bindings (asterix-rs/)
- Cross-platform builds (Windows/macOS/ARM64)
- Enhanced security scanning (CodeQL, cargo-audit)
- Pre-commit hooks
- Performance improvements (+60-70%)
- Extended test coverage (92.2%)

**Upstream Typically Changes:**
- New ASTERIX categories (CAT files)
- Bug fixes in parsing logic
- Version number updates
- Minor documentation updates

---

## Quick Command Reference

```bash
# Before merge: Clean history
java -jar /tmp/bfg-1.14.0.jar --delete-folders CMakeFiles .
git reflog expire --expire=now --all && git gc --prune=now --aggressive
git push --force origin master

# Fetch upstream
git fetch upstream

# See changes
git log --oneline master..upstream/master

# Merge
git merge --no-ff upstream/master

# Verify
cd src && make clean && make && make test
cd ../install/test && ./test.sh

# Update versions
vim debian/changelog asterix.spec .github/workflows/nightly-builds.yml

# Push
git push origin master
```

---

**Last Updated:** 2025-11-01
**Next Review:** Before next upstream sync
**Owner:** ASTERIX Project Maintainers
