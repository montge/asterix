# Change: Reorganize Repository Root Directory

## Why

The repository root contains 40+ files and directories, making it hard to navigate and understand the project structure. Build artifacts, documentation files, and configuration files are mixed together without clear organization.

## What Changes

### 1. Move Documentation to `docs/`
- `ASTERIX_CATEGORY_SUPPORT_MATRIX.md` → `docs/`
- `BUILD_COMMAND_REFERENCE.txt` → `docs/build/`
- `BUILD_WINDOWS.md` → `docs/build/`
- `CROSS_PLATFORM_BUILD_GUIDE.md` → `docs/build/`
- `CROSS_PLATFORM_TESTING.md` → `docs/testing/`
- `DO-278_IMPLEMENTATION_GUIDE.md` → already in `docs/`
- `DOCUMENTATION.md` → `docs/`
- `DOXYGEN.md` → `docs/`
- `GIT_HOOKS_SETUP.md` → `docs/development/`
- `INSTALLATION_GUIDE.md` → `docs/`
- `LANGUAGE_BINDINGS_COMPARISON.md` → `docs/bindings/`
- `PERFORMANCE_OPTIMIZATIONS.md` → `docs/`
- `PYTHON_BINDING_API.md` → `docs/bindings/`
- `BREAKING_CHANGES.md` → `docs/`

### 2. Add Build Artifacts to .gitignore
- `CMakeFiles/`
- `dataitembits_raw.info`
- `explicit_cov_detail/`
- `explicit_only.info`
- `htmlcov/`
- `lib/`
- `obj/`
- `valgrind.result`
- `.coverage`

### 3. Remove Legacy Files
- `.cproject` (Eclipse project file - not used)
- `.project` (Eclipse project file - not used)
- `.travis.yml` (Travis CI replaced by GitHub Actions)
- `index.html` (orphaned file)

### 4. Keep in Root (Standard Locations)
- `README.md`
- `LICENSE`
- `CONTRIBUTING.md`
- `CLAUDE.md` (AI assistant instructions)
- `AGENTS.md` (AI agent coordination)
- `HISTORY` (changelog)
- `CMakeLists.txt`
- `Doxyfile`
- `Dockerfile`
- `docker-compose.yml`
- `pyproject.toml`
- `setup.py`
- `setup.cfg`
- `requirements*.txt`
- `sonar-project.properties`
- `.gitignore`, `.gitattributes`
- `.pre-commit-config.yaml`
- `INSTALL_HOOKS.sh`
- `setup_dev_env.sh`

## Impact

- **Affected specs**: None (organizational change only)
- **Affected code**: None (only file locations change)
- **Documentation links**: Will need updating in README and docs
- **CI/CD**: May need path updates in workflows
- **Build system**: CMakeLists.txt may need path updates

## Risk Assessment

- **Low risk**: Moving documentation files
- **Medium risk**: Removing legacy files (verify not needed)
- **Low risk**: Adding to .gitignore (existing files stay)
