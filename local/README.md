# Local Development Directory

This directory is for **local-only development files** that should never be committed to git.

## Purpose

Use this directory for:
- **Testing scripts** - PowerShell, Bash, Python scripts for local testing
- **Build experiments** - Custom build configurations and outputs
- **Draft documentation** - Work-in-progress markdown files
- **Personal notes** - Development notes, TODO lists, debugging logs
- **Temporary data** - Test data, output files, logs
- **Custom configurations** - User-specific settings and paths

## What NOT to Put Here

- Source code that should be tracked (belongs in `src/`, `asterix/`, `asterix-rs/`)
- Official documentation (belongs in root or `docs/`)
- Test data that should be shared (belongs in `install/sample_data/`)
- Build scripts that are part of the project (belong in root or `.github/`)

## Important

The `local/` directory itself is ignored by git (see `.gitignore`), **except for this README.md**.

All contents you create here will remain on your machine only and will not be committed to the repository.

## Examples

```
local/
├── README.md              # This file (tracked in git)
├── my_test_script.ps1     # Your custom test script (not tracked)
├── build_experiments/     # Custom build configurations (not tracked)
├── notes.md               # Personal development notes (not tracked)
├── test_output.log        # Build/test logs (not tracked)
└── custom_configs/        # User-specific settings (not tracked)
```

## Security

Never store sensitive information (credentials, API keys, tokens) anywhere in the repository, including this directory. Use environment variables or secure credential managers instead.
