# ASTERIX C++ API Documentation

This directory contains the configuration and output for automatically generated C++ API documentation using Doxygen.

## Overview

The ASTERIX Decoder C++ API documentation is automatically generated from source code comments using Doxygen. The documentation covers:

- **src/asterix/** - ASTERIX protocol-specific implementation
- **src/engine/** - Generic data processing framework
- **src/main/** - Application entry points

## Documentation Features

- Class hierarchy diagrams
- Collaboration graphs
- Call graphs and caller graphs
- Include dependency graphs
- Source code browser
- Searchable interface
- Interactive navigation tree

## Building Documentation Locally

### Prerequisites

Install Doxygen and Graphviz (for generating diagrams):

```bash
# Ubuntu/Debian
sudo apt-get install doxygen graphviz

# macOS
brew install doxygen graphviz

# Fedora/RHEL
sudo dnf install doxygen graphviz
```

### Generate Documentation

From the repository root:

```bash
# Generate HTML documentation
doxygen Doxyfile

# Open in browser
xdg-open docs/cpp/html/index.html  # Linux
open docs/cpp/html/index.html      # macOS
```

The generated HTML will be placed in `docs/cpp/html/`.

## Automated GitHub Pages Deployment

Documentation is automatically built and deployed to GitHub Pages whenever:

1. C++ source files are modified (`.cpp`, `.cxx`, `.c`, `.h`, `.hxx`, `.hpp`)
2. The Doxyfile configuration is updated
3. Changes are pushed to the `master` branch

The workflow is defined in `.github/workflows/doxygen.yml`.

### Viewing Published Documentation

Once deployed, the documentation will be available at:
```
https://YOUR_USERNAME.github.io/asterix/
```

## Configuration

The Doxygen configuration is stored in `Doxyfile` at the repository root.

### Key Settings

- **PROJECT_NAME**: "ASTERIX Decoder C++ API"
- **PROJECT_NUMBER**: "2.8.9"
- **INPUT**: `src/asterix`, `src/engine`, `src/main`
- **OUTPUT_DIRECTORY**: `docs/cpp`
- **EXTRACT_ALL**: NO (only documented items shown)
- **EXTRACT_PRIVATE**: NO (public API only)
- **GENERATE_HTML**: YES
- **GENERATE_LATEX**: NO
- **SOURCE_BROWSER**: YES
- **HAVE_DOT**: YES (requires Graphviz)
- **CALL_GRAPH**: YES
- **CALLER_GRAPH**: YES

### Modifying Configuration

To customize the documentation:

1. Edit `Doxyfile` in the repository root
2. Test locally: `doxygen Doxyfile`
3. Commit changes to trigger automatic deployment

## Documentation Standards

For best results, use Doxygen-style comments in C++ source code:

```cpp
/**
 * @brief Brief description of the class/function
 *
 * Detailed description explaining the purpose and behavior.
 *
 * @param paramName Description of the parameter
 * @return Description of return value
 * @throws ExceptionType When this exception is thrown
 *
 * @note Additional notes or caveats
 * @warning Important warnings
 * @see RelatedClass
 */
```

## Troubleshooting

### Missing Graphs

If call graphs or class diagrams are not generated:

1. Verify Graphviz is installed: `dot -V`
2. Check `HAVE_DOT = YES` in Doxyfile
3. Ensure `DOT_PATH` is set correctly (or empty for system PATH)

### Documentation Not Appearing

1. Check that functions/classes have Doxygen-style comments (`/** ... */`)
2. Verify `EXTRACT_ALL = NO` requires documented items
3. Review Doxygen warnings during generation

### GitHub Pages Not Updating

1. Check the Actions tab for workflow status
2. Verify GitHub Pages is enabled in repository settings
3. Ensure the workflow has necessary permissions

## Directory Structure

```
docs/cpp/
├── .gitkeep          # Keeps directory in git
├── README.md         # This file
└── html/             # Generated HTML (gitignored)
    ├── index.html
    ├── classes.html
    ├── files.html
    └── ...
```

## License

The documentation is generated from source code licensed under GNU General Public License v3.0.
See the main repository LICENSE file for details.
