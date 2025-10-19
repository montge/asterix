# ReadTheDocs Setup Guide

This guide explains how to set up and publish the ASTERIX Decoder documentation on ReadTheDocs.org.

## Overview

The ASTERIX Decoder project now includes complete ReadTheDocs configuration for automatic documentation building and hosting. The documentation is built using Sphinx and uses the modern ReadTheDocs theme.

## Configuration Files

The following files have been created/updated for ReadTheDocs integration:

1. **`.readthedocs.yaml`** - Main ReadTheDocs configuration file
   - Uses ReadTheDocs config version 2
   - Ubuntu 22.04 build environment
   - Python 3.11
   - Builds the C extension with required system packages (libexpat1-dev, build-essential)
   - Generates HTML, PDF, and EPUB formats

2. **`requirements-docs.txt`** - Documentation build dependencies
   - Sphinx core (>=5.0.0)
   - sphinx-rtd-theme (>=1.2.0) - ReadTheDocs theme
   - All required Sphinx extensions

3. **`asterix/docs/conf.py`** - Updated Sphinx configuration
   - Dynamic version loading from `asterix/version.py`
   - ReadTheDocs theme enabled
   - Project metadata updated
   - Python path configured for autodoc

## How to Connect to ReadTheDocs

Follow these steps to publish the documentation:

### 1. Sign Up / Log In to ReadTheDocs

1. Go to [https://readthedocs.org/](https://readthedocs.org/)
2. Sign in with your GitHub account (recommended) or create an account

### 2. Import Your Project

1. Click on your username in the top-right corner
2. Select "My Projects"
3. Click "Import a Project"
4. If using GitHub integration:
   - Click "Import Manually" or select from your GitHub repositories
   - Choose the `asterix` repository
5. If importing manually:
   - Repository URL: `https://github.com/CroatiaControlLtd/asterix`
   - Or your fork's URL if working on a fork

### 3. Configure Project Settings

1. **Project Name**: `asterix-decoder` (or your preference)
2. **Repository URL**: Auto-filled if using GitHub
3. **Default Branch**: `master`
4. **Language**: English
5. **Programming Language**: Python

### 4. Initial Build

After importing, ReadTheDocs will automatically trigger a build. This can take 3-5 minutes.

## Expected Build Process

The ReadTheDocs build will:

1. Create a Python 3.11 virtual environment on Ubuntu 22.04
2. Install system dependencies: `libexpat1-dev`, `build-essential`, `python3-dev`
3. Install the ASTERIX package (compiling the C extension)
4. Install documentation dependencies from `requirements-docs.txt`
5. Run Sphinx to build HTML documentation
6. Generate PDF and EPUB versions

## Documentation URL

Once the build succeeds, your documentation will be available at:

- **HTML**: `https://asterix-decoder.readthedocs.io/en/latest/`
- **PDF**: `https://asterix-decoder.readthedocs.io/_/downloads/en/latest/pdf/`
- **EPUB**: `https://asterix-decoder.readthedocs.io/_/downloads/en/latest/epub/`

(Replace `asterix-decoder` with your actual project slug if different)

## Build Status Badge

Add this badge to your README.md to show documentation build status:

```markdown
[![Documentation Status](https://readthedocs.org/projects/asterix-decoder/badge/?version=latest)](https://asterix-decoder.readthedocs.io/en/latest/?badge=latest)
```

## Troubleshooting

### Build Fails During C Extension Compilation

**Symptoms**: Error during `pip install .` step mentioning compilation errors

**Solutions**:
- Verify `libexpat1-dev` is listed in `.readthedocs.yaml` under `build.apt_packages`
- Check that `build-essential` and `python3-dev` are included
- Review the build log for specific compilation errors

### Module Import Errors

**Symptoms**: `autodoc` fails with "No module named 'asterix'"

**Solutions**:
- Ensure `pip install .` is listed in `.readthedocs.yaml` before the requirements
- Verify `sys.path.insert(0, os.path.abspath('../..'))` is in `conf.py`
- The C extension must build successfully for the module to be importable

### Missing Dependencies

**Symptoms**: Build fails with "No module named 'sphinx_rtd_theme'"

**Solutions**:
- Check that `requirements-docs.txt` includes all required packages
- Verify the requirements file is referenced in `.readthedocs.yaml`
- Ensure package versions are compatible

### Version Not Showing Correctly

**Symptoms**: Documentation shows version as "1.0" instead of current version

**Solutions**:
- Verify `asterix/version.py` exists and contains `__version__` variable
- Check the version parsing code in `conf.py`
- Review build logs for any errors during version loading

### Theme Not Applied

**Symptoms**: Documentation uses default Alabaster theme instead of ReadTheDocs theme

**Solutions**:
- Verify `sphinx-rtd-theme` is in `requirements-docs.txt`
- Check that `html_theme = 'sphinx_rtd_theme'` is set in `conf.py`
- Clear the build cache on ReadTheDocs: Project Admin → Advanced Settings → Wipe

## Local Testing

Before pushing to ReadTheDocs, test the documentation build locally:

```bash
# Install documentation dependencies
pip install -r requirements-docs.txt

# Install the package (builds C extension)
pip install .

# Build documentation
cd asterix/docs
make html

# View the output
# HTML files will be in asterix/docs/_build/html/
# Open asterix/docs/_build/html/index.html in your browser
```

## Automatic Builds

ReadTheDocs is configured to automatically rebuild documentation when:

- Commits are pushed to the `master` branch
- Pull requests are created (builds preview)
- Tags are created (creates versioned documentation)

You can also trigger manual builds from the ReadTheDocs dashboard.

## Advanced Configuration

### Version Management

To enable multi-version documentation:

1. Go to Project Admin → Versions
2. Activate versions for specific tags or branches
3. Set the default version (usually "latest" or "stable")

### Custom Domain

To use a custom domain (e.g., `docs.asterix-decoder.org`):

1. Go to Project Admin → Domains
2. Add your custom domain
3. Configure DNS CNAME record pointing to ReadTheDocs

### Webhook Configuration

If automatic builds aren't triggering:

1. Go to Project Admin → Integrations
2. Copy the webhook URL
3. Add it to your GitHub repository webhooks
4. Select "Just the push event"

## Support

For ReadTheDocs-specific issues:
- ReadTheDocs documentation: https://docs.readthedocs.io/
- Community support: https://readthedocs.org/support/

For ASTERIX Decoder documentation issues:
- Create an issue on GitHub: https://github.com/CroatiaControlLtd/asterix/issues

## Summary

The documentation is now ready for ReadTheDocs! The configuration handles:

- ✅ C extension compilation with required system dependencies
- ✅ Dynamic version loading from source
- ✅ Modern ReadTheDocs theme
- ✅ Multi-format output (HTML, PDF, EPUB)
- ✅ Automatic builds on push

Simply import the project on ReadTheDocs.org and the documentation will build automatically.
