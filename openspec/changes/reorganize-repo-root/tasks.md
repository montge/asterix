## 1. Preparation

- [ ] 1.1 Create backup branch before reorganization
- [ ] 1.2 Verify no pending PRs that would be affected
- [ ] 1.3 Document current file locations

## 2. Update .gitignore

- [ ] 2.1 Add `CMakeFiles/` to .gitignore
- [ ] 2.2 Add `*.info` coverage files to .gitignore
- [ ] 2.3 Add `htmlcov/` to .gitignore
- [ ] 2.4 Add `lib/` and `obj/` to .gitignore
- [ ] 2.5 Add `valgrind.result` to .gitignore
- [ ] 2.6 Remove tracked build artifacts from git

## 3. Create Documentation Structure

- [ ] 3.1 Create `docs/build/` directory
- [ ] 3.2 Create `docs/testing/` directory
- [ ] 3.3 Create `docs/development/` directory
- [ ] 3.4 Create `docs/bindings/` directory

## 4. Move Documentation Files

- [ ] 4.1 Move `ASTERIX_CATEGORY_SUPPORT_MATRIX.md` → `docs/`
- [ ] 4.2 Move `BUILD_COMMAND_REFERENCE.txt` → `docs/build/`
- [ ] 4.3 Move `BUILD_WINDOWS.md` → `docs/build/`
- [ ] 4.4 Move `CROSS_PLATFORM_BUILD_GUIDE.md` → `docs/build/`
- [ ] 4.5 Move `CROSS_PLATFORM_TESTING.md` → `docs/testing/`
- [ ] 4.6 Move `DOCUMENTATION.md` → `docs/`
- [ ] 4.7 Move `DOXYGEN.md` → `docs/`
- [ ] 4.8 Move `GIT_HOOKS_SETUP.md` → `docs/development/`
- [ ] 4.9 Move `INSTALLATION_GUIDE.md` → `docs/`
- [ ] 4.10 Move `LANGUAGE_BINDINGS_COMPARISON.md` → `docs/bindings/`
- [ ] 4.11 Move `PERFORMANCE_OPTIMIZATIONS.md` → `docs/`
- [ ] 4.12 Move `PYTHON_BINDING_API.md` → `docs/bindings/`
- [ ] 4.13 Move `BREAKING_CHANGES.md` → `docs/`

## 5. Remove Legacy Files

- [ ] 5.1 Verify `.cproject` is not used, then remove
- [ ] 5.2 Verify `.project` is not used, then remove
- [ ] 5.3 Verify `.travis.yml` is not used, then remove
- [ ] 5.4 Verify `index.html` is orphaned, then remove

## 6. Update References

- [ ] 6.1 Update README.md with new documentation paths
- [ ] 6.2 Update CLAUDE.md with new file locations
- [ ] 6.3 Update any CI workflows with changed paths
- [ ] 6.4 Update docs/ internal cross-references
- [ ] 6.5 Create docs/README.md index file

## 7. Verification

- [ ] 7.1 Run full test suite (C++, Python, Rust)
- [ ] 7.2 Verify documentation builds correctly
- [ ] 7.3 Verify CI workflows pass
- [ ] 7.4 Test build from clean clone
- [ ] 7.5 Update docs navigation/index

## Progress Tracking

| Phase | Tasks | Completed |
|-------|-------|-----------|
| Preparation | 3 | 0 |
| .gitignore | 6 | 0 |
| Docs Structure | 4 | 0 |
| Move Files | 13 | 0 |
| Remove Legacy | 4 | 0 |
| Update References | 5 | 0 |
| Verification | 5 | 0 |
| **Total** | **40** | **0** |
