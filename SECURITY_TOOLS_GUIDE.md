# Security Tools Guide - ASTERIX Project

This document clarifies which security tools are used in the ASTERIX project, distinguishing between **FREE tools** (used by default) and **optional commercial tools** (if available).

## TL;DR

**Default Setup (100% FREE)**:
- All security features work out-of-the-box with free, open-source tools
- No commercial licenses required
- Full CI/CD pipeline uses only free tools

**Optional Enhancements**:
- Commercial tools (HP Fortify, Coverity) can be added if you have licenses
- Not required, just complementary

---

## 1. Compiler Hardening Flags (FREE, Always Enabled)

### `-D_FORTIFY_SOURCE=2` ⚠️ **NOT** Commercial HP Fortify!

**What it is**: Built-in GCC/glibc compile-time buffer overflow detection
**Cost**: **FREE** (included with GCC/glibc)
**How it works**: Replaces unsafe functions (`strcpy`, `sprintf`) with safer versions that check buffer sizes at runtime

**Enabled in**: `src/makefile.include`
```makefile
CFLAGS = -D_FORTIFY_SOURCE=2 -fstack-protector-strong -Wl,-z,relro,-z,now
```

**This is NOT**:
- ❌ HP Fortify Static Code Analyzer (commercial tool, ~$10k-50k/year)
- ❌ Micro Focus Fortify SCA (same thing, rebranded)

**Benefits**:
- Detects buffer overflows at compile-time and runtime
- Zero performance overhead in most cases
- Widely used in production systems (Android, Chrome, Firefox)

### `-fstack-protector-strong`

**What it is**: GCC stack canary protection
**Cost**: **FREE**
**How it works**: Inserts "canary" values on the stack to detect buffer overflows

### `-Wl,-z,relro,-z,now`

**What it is**: Linker hardening (Full RELRO)
**Cost**: **FREE**
**How it works**: Makes the Global Offset Table (GOT) read-only to prevent GOT overwrite attacks

---

## 2. Static Analysis Tools

### **Primary (FREE, Default)**

| Tool | Purpose | Cost | Used In |
|------|---------|------|---------|
| **CodeQL** | Semantic code analysis (C++, Python) | FREE for OSS | GitHub Actions CI |
| **cppcheck** | C/C++ static analysis | FREE | CI + pre-push hook |
| **flake8** | Python linting | FREE | pre-commit hook |
| **cpplint** | C++ style checking | FREE | pre-commit hook |
| **mypy** | Python type checking | FREE | pre-push hook |

**Setup**: Already integrated in CI pipeline and git hooks, zero configuration needed.

### **Optional Commercial Tools**

| Tool | Purpose | Cost | How to Enable |
|------|---------|------|---------------|
| **HP Fortify SCA** | Enterprise static analysis | ~$10k-50k/year | See section 4 |
| **Coverity** | Advanced static analysis | ~$15k-30k/year | See section 5 |
| **SonarQube** | Code quality platform | Enterprise: ~$12k/year | See section 6 |

**These are OPTIONAL** - only use if your organization already has licenses.

---

## 3. Dynamic Analysis & Runtime Tools

### **FREE Tools (Default)**

| Tool | Purpose | Cost | Used In |
|------|---------|------|---------|
| **Valgrind** | Memory leak detection | FREE | CI + integration tests |
| **AddressSanitizer** | Memory error detector | FREE (LLVM) | Optional build target |
| **UndefinedBehaviorSanitizer** | UB detection | FREE (LLVM) | Optional build target |

**Setup**:
```bash
# Valgrind (already integrated)
cd install/test && ./valgrind_test.sh

# AddressSanitizer (optional)
cd src && ASAN=1 make clean && make
```

---

## 4. Enabling HP Fortify SCA (Optional, If Licensed)

**Prerequisites**:
- Active HP Fortify SCA license
- Fortify installed on CI runners or local machine

### Local Scan

```bash
# 1. Translate source code
sourceanalyzer -b asterix-build \
  -clean \
  -D_FORTIFY_SOURCE=2 \
  -std=c++17 \
  src/**/*.cpp src/**/*.c

# 2. Run analysis
sourceanalyzer -b asterix-build \
  -scan \
  -f fortify_results.fpr

# 3. Generate report
ReportGenerator -format pdf \
  -f fortify_report.pdf \
  -source fortify_results.fpr
```

### CI Integration (GitHub Actions)

Create `.github/workflows/fortify-scan.yml`:

```yaml
name: HP Fortify Scan

on:
  schedule:
    - cron: '0 2 * * 1'  # Weekly Monday 2am
  workflow_dispatch:

jobs:
  fortify:
    runs-on: self-hosted  # Requires licensed runner
    steps:
      - uses: actions/checkout@v4

      - name: Run Fortify SCA
        run: |
          sourceanalyzer -b asterix -clean
          sourceanalyzer -b asterix src/**/*.{cpp,c}
          sourceanalyzer -b asterix -scan -f results.fpr

      - name: Upload Results
        uses: actions/upload-artifact@v4
        with:
          name: fortify-results
          path: results.fpr
```

**Note**: Only enable this if you have a Fortify license. The free tools provide 90% of the same value.

---

## 5. Enabling Coverity (Optional, If Licensed)

**Prerequisites**:
- Coverity Scan account (free for OSS projects: https://scan.coverity.com)
- OR Synopsys Coverity license (commercial)

### Coverity Scan (FREE for Open Source)

```bash
# 1. Download Coverity Build Tool
wget https://scan.coverity.com/download/cxx/linux64 \
  --post-data "token=${COVERITY_TOKEN}&project=asterix" \
  -O coverity_tool.tgz
tar xzf coverity_tool.tgz

# 2. Build with Coverity
cd src
../cov-analysis/bin/cov-build --dir cov-int make clean all

# 3. Submit results
tar czvf asterix.tgz cov-int
curl --form token=${COVERITY_TOKEN} \
  --form email=your@email.com \
  --form file=@asterix.tgz \
  --form version="$(git rev-parse --short HEAD)" \
  https://scan.coverity.com/builds?project=asterix
```

**Free for OSS**: Register project at https://scan.coverity.com

---

## 6. Enabling SonarQube (Optional)

**Options**:
- SonarCloud (FREE for public repos): https://sonarcloud.io
- SonarQube Community Edition (FREE self-hosted)
- SonarQube Enterprise (commercial)

### SonarCloud Setup (FREE)

```yaml
# .github/workflows/sonarcloud.yml
name: SonarCloud Scan

on:
  push:
    branches: [master]
  pull_request:

jobs:
  sonar:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          fetch-depth: 0

      - name: SonarCloud Scan
        uses: SonarSource/sonarcloud-github-action@master
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
          SONAR_TOKEN: ${{ secrets.SONAR_TOKEN }}
```

---

## 7. Dependency Scanning

### **FREE Tools (Default)**

| Tool | Purpose | Status |
|------|---------|--------|
| **Dependabot** | Automated dependency updates | ✅ Active |
| **GitHub Security Advisories** | CVE monitoring | ✅ Active |
| **pip-audit** | Python dependency scanning | ✅ Available |

**Already configured**: `.github/dependabot.yml`

### **Optional Commercial**

- Snyk (freemium, commercial for teams)
- WhiteSource Bolt (commercial)
- JFrog Xray (commercial)

---

## 8. Summary Matrix

| Feature | FREE Tools | Commercial Alternative | Recommendation |
|---------|------------|------------------------|----------------|
| **Compiler Hardening** | `-D_FORTIFY_SOURCE=2` <br> `-fstack-protector-strong` | N/A | ✅ **Use FREE** (enabled by default) |
| **Static Analysis** | CodeQL, cppcheck | HP Fortify, Coverity | ✅ **Use FREE** unless you have licenses |
| **Memory Checking** | Valgrind, ASan | Purify, BoundsChecker | ✅ **Use FREE** |
| **Dependency Scanning** | Dependabot, pip-audit | Snyk, WhiteSource | ✅ **Use FREE** |
| **Code Quality** | SonarCloud (OSS) | SonarQube Enterprise | ✅ **Use FREE** for OSS projects |

---

## 9. Recommendation for This Project

**Current Setup (100% FREE)**:
```
✅ Compiler hardening (-D_FORTIFY_SOURCE=2, stack protector, RELRO)
✅ CodeQL scanning (GitHub native)
✅ Dependabot (automated updates)
✅ cppcheck (static analysis)
✅ Valgrind (memory checks)
✅ Pre-commit hooks (secret detection, linting)
```

**This provides**:
- 90% of commercial tool value
- Zero licensing costs
- Full CI/CD automation
- Production-ready security

**Add commercial tools ONLY if**:
- Your organization already has licenses
- Compliance requires specific tools (aerospace, medical, etc.)
- You need advanced features (deep dataflow analysis, custom rules)

---

## 10. FAQs

### Q: Do I need HP Fortify to use `-D_FORTIFY_SOURCE=2`?

**A**: **NO!** `-D_FORTIFY_SOURCE=2` is a FREE GCC/glibc feature. HP Fortify SCA is a completely separate commercial product that happens to have a similar name.

### Q: What's the difference?

**A**:
- `-D_FORTIFY_SOURCE=2`: FREE compiler flag, checks buffer sizes at runtime
- HP Fortify SCA: $10k-50k/year static analyzer, finds vulnerabilities in source code

They are complementary, not the same thing.

### Q: Should I buy commercial tools?

**A**: Only if:
1. You already have licenses (corporate environment)
2. Compliance mandates specific tools
3. Free tools don't meet your specific needs

For most projects, the free tools are sufficient.

### Q: Can I run both free and commercial tools?

**A**: Absolutely! They complement each other:
- Use FREE tools for everyday CI/CD
- Run commercial tools weekly/monthly for deeper analysis

---

## 11. Getting Help

**FREE tool issues**:
- CodeQL: https://github.com/github/codeql/discussions
- cppcheck: https://github.com/danmar/cppcheck/issues
- Valgrind: https://valgrind.org/support.html

**Commercial tool support**:
- HP Fortify: Contact your account manager
- Coverity: Synopsys support portal
- SonarQube: SonarSource support

---

## Conclusion

**You don't need ANY commercial tools to have excellent security**. The ASTERIX project uses 100% free, open-source tools by default and achieves production-grade security:

- ✅ 41 CVEs fixed
- ✅ Zero known active vulnerabilities
- ✅ Compiler hardening enabled
- ✅ Automated scanning in CI
- ✅ Memory leak detection
- ✅ Dependency monitoring

Commercial tools are optional enhancements, not requirements.

---

**Last Updated**: 2025-11-01
**Maintainer**: ASTERIX Project Team
