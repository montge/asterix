# Cross-Platform Build System - Documentation Index

Complete cross-platform build system for the ASTERIX decoder project supporting Windows, macOS, and Linux.

## Quick Start

**New to the project?** Start here:
1. Read [CROSS_PLATFORM_SUMMARY.md](CROSS_PLATFORM_SUMMARY.md) for project overview
2. Follow [CROSS_PLATFORM_IMPLEMENTATION_CHECKLIST.md](CROSS_PLATFORM_IMPLEMENTATION_CHECKLIST.md) step-by-step
3. Reference [CROSS_PLATFORM_BUILD_GUIDE.md](CROSS_PLATFORM_BUILD_GUIDE.md) for platform-specific builds

## Documentation Files

### 1. CROSS_PLATFORM_SUMMARY.md
**Purpose**: Executive overview and project status
**Size**: 589 lines (24 KB)
**Audience**: Project managers, stakeholders, developers

**Contents**:
- Deliverables summary
- Platform support matrix
- Dependencies and requirements
- Success criteria and risks
- Implementation roadmap

**When to use**: First time reading about the cross-platform system, or getting high-level project status.

---

### 2. CROSS_PLATFORM_IMPLEMENTATION_CHECKLIST.md
**Purpose**: Step-by-step implementation guide
**Size**: 640 lines (20 KB)
**Audience**: Developers implementing the changes

**Contents**:
- Phase 1: Enable CI/CD (30 min)
- Phase 2: Fix Windows build (2-3 hours)
- Phase 3: Local testing (1 hour per platform)
- Phase 4: Python module testing
- Phase 5: Verify CI/CD success
- Phase 6: Documentation updates

**When to use**: Actively implementing cross-platform support, need exact code changes and commands.

---

### 3. CROSS_PLATFORM_BUILD_GUIDE.md
**Purpose**: Comprehensive platform-specific build instructions
**Size**: 803 lines (20 KB)
**Audience**: End users, developers building from source

**Contents**:
- Windows build (CMake + MSVC, Visual Studio IDE, MSI creation)
- macOS build (CMake, Homebrew, DMG creation, code signing)
- Linux build (CMake, Make, DEB/RPM packages)
- Python module build (all platforms)
- Known platform issues and workarounds
- Testing procedures

**When to use**: Building the project on a specific platform, troubleshooting build issues.

---

### 4. CROSS_PLATFORM_CODE_CHANGES.md
**Purpose**: Detailed code modification requirements
**Size**: 563 lines (16 KB)
**Audience**: Developers making code changes

**Contents**:
- Priority 1: Critical changes (CMakeLists.txt, setup.py)
- Priority 2: Networking code abstraction (socket headers, Winsock2)
- Priority 3: Optional enhancements (serial devices, path handling)
- Testing strategy
- Implementation roadmap (6-week plan)

**When to use**: Understanding what code needs to change and why, planning refactoring work.

---

### 5. CROSS_PLATFORM_TESTING.md
**Purpose**: Testing procedures and troubleshooting
**Size**: 730 lines (20 KB)
**Audience**: QA engineers, developers testing builds

**Contents**:
- Test suite overview
- Windows testing (PowerShell script, manual tests)
- macOS testing (shell script, Instruments, valgrind)
- Linux testing (shell script, valgrind, code coverage)
- Python module testing (pytest, unittest)
- CI/CD verification
- Troubleshooting guide

**When to use**: Running tests on any platform, investigating test failures, setting up CI/CD.

---

### 6. .github/workflows/cross-platform-builds.yml
**Purpose**: Automated CI/CD workflow
**Size**: 534 lines (20 KB)
**Audience**: DevOps engineers, CI/CD maintainers

**Contents**:
- Windows build jobs (2019, 2022 × Release, Debug)
- macOS build jobs (13 Intel, 14 ARM × Release, Debug)
- Linux build jobs (22.04, 24.04 × Release, Debug)
- Python module jobs (3.9-3.13 × 3 platforms)
- Build summary and artifact collection

**When to use**: Setting up GitHub Actions, debugging CI failures, understanding build matrix.

---

### 7. install/test/test.ps1
**Purpose**: Windows PowerShell test script
**Size**: 313 lines (12 KB)
**Audience**: Windows testers, developers

**Contents**:
- Automated test suite for Windows
- Basic execution tests (help, version, filters)
- File parsing tests (PCAP, raw, GPS)
- Output format comparison tests
- Color-coded results and summary

**When to use**: Testing ASTERIX builds on Windows (equivalent to Unix test.sh).

---

## File Relationships

```
CROSS_PLATFORM_SUMMARY.md
    └─> Overview of entire system
         │
         ├─> CROSS_PLATFORM_IMPLEMENTATION_CHECKLIST.md
         │   └─> Step-by-step instructions
         │        │
         │        ├─> CROSS_PLATFORM_CODE_CHANGES.md
         │        │   └─> Detailed code modifications
         │        │
         │        └─> CROSS_PLATFORM_BUILD_GUIDE.md
         │            └─> Platform build instructions
         │                 │
         │                 └─> CROSS_PLATFORM_TESTING.md
         │                     └─> Testing procedures
         │
         └─> .github/workflows/cross-platform-builds.yml
             └─> CI/CD automation
                  │
                  └─> install/test/test.ps1
                      └─> Windows test script
```

---

## Documentation Statistics

| File                                  | Lines | Size  | Words  | Purpose                  |
|---------------------------------------|-------|-------|--------|--------------------------|
| CROSS_PLATFORM_SUMMARY.md             | 589   | 24 KB | ~4,800 | Executive overview       |
| CROSS_PLATFORM_IMPLEMENTATION_CHECKLIST.md | 640 | 20 KB | ~5,200 | Implementation guide    |
| CROSS_PLATFORM_BUILD_GUIDE.md         | 803   | 20 KB | ~6,500 | Build instructions       |
| CROSS_PLATFORM_CODE_CHANGES.md        | 563   | 16 KB | ~4,600 | Code modifications       |
| CROSS_PLATFORM_TESTING.md             | 730   | 20 KB | ~5,900 | Testing procedures       |
| cross-platform-builds.yml             | 534   | 20 KB | ~4,300 | CI/CD workflow           |
| test.ps1                              | 313   | 12 KB | ~2,500 | Windows test script      |
| **Total**                             | **4,172** | **132 KB** | **~33,800** | **Complete system** |

---

## Reading Paths by Role

### Project Manager / Stakeholder
1. **CROSS_PLATFORM_SUMMARY.md** - Get project overview
2. **Platform Support Matrix** - Understand capabilities
3. **Success Criteria** - Verify completion status

### Developer (Implementing Changes)
1. **CROSS_PLATFORM_IMPLEMENTATION_CHECKLIST.md** - Follow steps
2. **CROSS_PLATFORM_CODE_CHANGES.md** - Understand modifications
3. **CROSS_PLATFORM_TESTING.md** - Verify changes work

### End User (Building from Source)
1. **CROSS_PLATFORM_BUILD_GUIDE.md** - Follow platform instructions
2. **Platform-specific sections** (Windows/macOS/Linux)
3. **Troubleshooting** - Resolve build issues

### QA Engineer / Tester
1. **CROSS_PLATFORM_TESTING.md** - Test procedures
2. **Platform-specific testing** (Windows/macOS/Linux)
3. **CI/CD verification** - Check automated tests

### DevOps Engineer
1. **cross-platform-builds.yml** - CI/CD workflow
2. **CROSS_PLATFORM_TESTING.md** - Integration tests
3. **CROSS_PLATFORM_BUILD_GUIDE.md** - Dependency setup

---

## Supported Platforms

### Operating Systems
- **Windows**: 10, 11, Server 2019, Server 2022
- **macOS**: 11.0+ (Big Sur and later), Intel x86_64 and ARM64 (M1/M2/M3)
- **Linux**: Ubuntu 22.04+, Debian 11+, RHEL 8+, Fedora 38+, Rocky Linux 8+

### Compilers
- **MSVC**: 2019 (v142, C++17), 2022 (v143, C++23)
- **GCC**: 11 (C++17), 13+ (C++23)
- **AppleClang**: 15+ (C++23)

### Python
- **Versions**: 3.9, 3.10, 3.11, 3.12, 3.13
- **Platforms**: Windows, macOS (Intel/ARM), Linux

### Build Systems
- **CMake**: 3.12+ (3.20+ recommended)
- **Make**: GNU Make 4.0+ (Unix only)
- **Visual Studio**: 2019/2022 (Windows only)

---

## Implementation Status

### Completed ✅
- [x] Cross-platform workflow (GitHub Actions)
- [x] Build documentation (all platforms)
- [x] Code changes documentation
- [x] Testing procedures documentation
- [x] Windows test script (PowerShell)
- [x] Implementation checklist

### In Progress 🚧
- [ ] Code changes implementation (Priority 1)
- [ ] Local testing on all platforms
- [ ] CI/CD verification

### Planned 📋
- [ ] Networking abstraction (Winsock2)
- [ ] Installer creation (MSI, DMG)
- [ ] Serial device support (Windows)
- [ ] Performance benchmarking

---

## Quick Reference

### Build Commands

**Windows (PowerShell)**:
```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
cmake --install build --config Release
```

**macOS (Terminal)**:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --parallel $(sysctl -n hw.ncpu)
cmake --install build
```

**Linux (Bash)**:
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel $(nproc)
cmake --install build
```

### Test Commands

**Windows**: `.\install\test\test.ps1`
**macOS/Linux**: `./install/test/test.sh`
**Python**: `pytest asterix/test/ -v --cov=asterix`

### CI/CD Workflow

**Trigger**: Push to master, develop, feature/**, or manual dispatch
**URL**: `https://github.com/{user}/asterix/actions`
**Matrix**: 40+ build/test combinations
**Artifacts**: Downloadable executables, libraries, and installers

---

## Getting Help

### Build Issues
- Check **CROSS_PLATFORM_BUILD_GUIDE.md** → Platform-specific section → Known Issues
- Review **CROSS_PLATFORM_TESTING.md** → Troubleshooting section
- Search GitHub Issues: https://github.com/{user}/asterix/issues

### Code Questions
- Read **CROSS_PLATFORM_CODE_CHANGES.md** → Detailed explanations
- Check existing code comments and documentation
- Open a discussion: https://github.com/{user}/asterix/discussions

### Testing Problems
- Consult **CROSS_PLATFORM_TESTING.md** → Platform-specific testing
- Run with verbose output: `./test.sh` or `.\test.ps1 -Verbose`
- Check CI logs on GitHub Actions

---

## Contributing

When making cross-platform changes:

1. **Test on all platforms** (or request CI verification)
2. **Update documentation** if behavior changes
3. **Follow coding standards** (C++17/23, security flags)
4. **Run test suites** before committing
5. **Verify CI passes** after pushing

---

## License

This documentation is part of the ASTERIX decoder project and is licensed under the GNU General Public License v3.0 or later.

See LICENSE file for details.

---

## Changelog

### 2025-11-01 - Version 1.0
- Initial release of cross-platform build system
- Complete documentation suite (4,172 lines)
- GitHub Actions workflow (40+ build combinations)
- Windows PowerShell test script
- Implementation checklist and roadmap

---

## Contact

- **GitHub**: https://github.com/montge/asterix
- **Issues**: https://github.com/montge/asterix/issues
- **Discussions**: https://github.com/montge/asterix/discussions

---

**Documentation Version**: 1.0
**Last Updated**: 2025-11-01
**Total Documentation Size**: 132 KB (4,172 lines, ~33,800 words)
