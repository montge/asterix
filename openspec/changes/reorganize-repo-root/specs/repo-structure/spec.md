## ADDED Requirements

### Requirement: Clean Repository Root
The repository root SHALL contain only essential project files (build configs, package manifests, CI configs) with all documentation organized in the `docs/` directory.

#### Scenario: Developer clones repository
- **WHEN** a developer clones the repository
- **THEN** the root directory contains fewer than 25 visible files/directories
- **AND** documentation is discoverable via `docs/` directory
- **AND** build artifacts are not tracked in git

### Requirement: Documentation Organization
The `docs/` directory SHALL be organized into logical subdirectories for different documentation types.

#### Scenario: Finding build documentation
- **WHEN** a developer needs build instructions
- **THEN** they find them in `docs/build/`

#### Scenario: Finding binding documentation
- **WHEN** a developer needs language binding documentation
- **THEN** they find them in `docs/bindings/`

### Requirement: Build Artifact Exclusion
Build artifacts and generated files SHALL be excluded from version control via `.gitignore`.

#### Scenario: After building locally
- **WHEN** a developer builds the project locally
- **THEN** generated files (CMakeFiles, *.info, htmlcov) are not staged for commit
- **AND** `git status` does not show build artifacts

## REMOVED Requirements

### Requirement: Legacy IDE Support
Eclipse project files (`.cproject`, `.project`) SHALL be removed as they are no longer maintained.

**Reason**: Project has moved to CMake-based development; VS Code and CLion use CMakeLists.txt directly.
**Migration**: Developers using Eclipse can generate project files from CMake.

### Requirement: Travis CI Configuration
The `.travis.yml` file SHALL be removed as CI has migrated to GitHub Actions.

**Reason**: GitHub Actions provides better integration with the repository.
**Migration**: All CI functionality is available in `.github/workflows/`.
