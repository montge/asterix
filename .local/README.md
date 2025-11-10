# .local/ Development Directory

This hidden directory is for **local-only development files** that should never be committed to git.

## Purpose

Use this directory for:
- **Testing scripts** - PowerShell, Bash, Python scripts for local testing
- **Build experiments** - Custom build configurations and outputs
- **Draft documentation** - Work-in-progress markdown files
- **Personal notes** - Development notes, TODO lists, debugging logs
- **Temporary data** - Test data, output files, logs
- **Custom configurations** - User-specific settings and paths
- **AI-generated content** - Code analysis reports, security findings, performance metrics
- **Coverage reports** - HTML/JSON coverage outputs, benchmark results
- **Experimental work** - Prototypes, temporary scripts, visualizations

## Recommended Structure

Organize your `.local/` directory for clarity:

```
.local/
├── README.md              # This file (tracked in git)
├── analysis/              # AI-generated code analysis reports
├── security/              # Security audit findings
├── benchmarks/            # Performance testing results
├── coverage/              # Code coverage reports (HTML/JSON)
├── experiments/           # Prototype code and experiments
├── logs/                  # Build and test logs
├── notes/                 # Personal development notes
├── scripts/               # Custom testing scripts
└── data/                  # Test data files
```

## What NOT to Put Here

- Source code that should be tracked (belongs in `src/`, `asterix/`, `asterix-rs/`)
- Official documentation (belongs in root or `docs/`)
- Test data that should be shared (belongs in `install/sample_data/`)
- Build scripts that are part of the project (belong in root or `.github/`)

## Important

The `.local/` directory itself is ignored by git (see `.gitignore`), **except for this README.md**.

All contents you create here will remain on your machine only and will not be committed to the repository.

## AI-Assisted Development

When using Claude Code or other AI tools, store generated artifacts here:

```bash
# Store AI analysis reports
.local/analysis/20251110_code_review.md
.local/analysis/20251110_security_scan.md

# Store coverage reports
.local/coverage/python_coverage_20251110.html
.local/coverage/cpp_coverage_20251110.info

# Store performance benchmarks
.local/benchmarks/parse_performance_20251110.txt

# Store experimental code
.local/experiments/new_parser_approach/
```

## Security

Never store sensitive information (credentials, API keys, tokens) anywhere in the repository, including this directory. Use environment variables or secure credential managers instead.

## See Also

- [CLAUDE.md](../CLAUDE.md) - Guidelines for Claude Code usage
- [CONTRIBUTING.md](../CONTRIBUTING.md) - Contribution guidelines
