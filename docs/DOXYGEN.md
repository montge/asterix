# C++ API Documentation with Doxygen

## Quick Start

### Generate Documentation Locally

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install doxygen graphviz

# Generate HTML documentation
doxygen Doxyfile

# Open documentation
xdg-open docs/cpp/html/index.html
```

### View Online Documentation

After pushing to GitHub, documentation is automatically built and deployed to GitHub Pages:

**URL**: `https://YOUR_USERNAME.github.io/asterix/`

## What Gets Documented

The Doxygen configuration generates API documentation for:

- **src/asterix/** - ASTERIX protocol implementation (DataItem, Category, parsers)
- **src/engine/** - Generic device/format framework (BaseDevice, BaseFormat, Channel)
- **src/main/** - Application entry points

## Documentation Features

- Class inheritance diagrams
- Collaboration graphs showing class relationships
- Call graphs (which functions call this function)
- Caller graphs (which functions are called by this function)
- Include dependency graphs
- Source code cross-referencing
- Full-text search

## Configuration Files

| File | Purpose |
|------|---------|
| `Doxyfile` | Main Doxygen configuration |
| `.github/workflows/doxygen.yml` | GitHub Actions workflow for auto-deployment |
| `docs/cpp/README.md` | Detailed documentation guide |
| `docs/cpp/.gitkeep` | Placeholder to keep directory in git |
| `.gitignore` | Excludes `docs/cpp/html/` from version control |

## Automatic Deployment

Documentation is rebuilt and deployed when:

1. Any C++ source file changes (`.cpp`, `.cxx`, `.c`, `.h`, `.hxx`, `.hpp`)
2. The `Doxyfile` is modified
3. Changes are pushed to the `master` branch

The GitHub Actions workflow:
- Installs Doxygen and Graphviz
- Generates HTML documentation
- Deploys to GitHub Pages

## Documentation Standards

For functions, classes, and methods to appear in the documentation, use Doxygen comment style:

```cpp
/**
 * @brief Parse ASTERIX data block
 *
 * This function parses a complete ASTERIX data block and extracts
 * all data records according to the category definition.
 *
 * @param data Pointer to raw ASTERIX data
 * @param length Length of data in bytes
 * @return Number of records parsed, or -1 on error
 */
int parseDataBlock(const unsigned char* data, size_t length);
```

### Supported Tags

- `@brief` - Short one-line description
- `@param` - Parameter description
- `@return` - Return value description
- `@throws` - Exceptions that may be thrown
- `@note` - Additional notes
- `@warning` - Important warnings
- `@see` - Cross-references to related items
- `@deprecated` - Mark deprecated APIs

## Key Configuration Settings

| Setting | Value | Description |
|---------|-------|-------------|
| `PROJECT_NAME` | "ASTERIX Decoder C++ API" | Documentation title |
| `PROJECT_NUMBER` | "2.8.9" | Version number |
| `EXTRACT_ALL` | NO | Only document items with Doxygen comments |
| `EXTRACT_PRIVATE` | NO | Exclude private members (public API only) |
| `SOURCE_BROWSER` | YES | Include source code browser |
| `HAVE_DOT` | YES | Enable Graphviz diagrams |
| `CALL_GRAPH` | YES | Generate call graphs |
| `CALLER_GRAPH` | YES | Generate caller graphs |

## Modifying Configuration

To customize documentation generation:

1. Edit `Doxyfile` configuration
2. Test locally: `doxygen Doxyfile`
3. Review output in `docs/cpp/html/index.html`
4. Commit and push to deploy to GitHub Pages

## Troubleshooting

### No graphs appear

Install Graphviz: `sudo apt-get install graphviz`

### Function not documented

Ensure it has a Doxygen comment block (`/** ... */`)

### GitHub Pages not updating

Check the Actions tab in GitHub for build status

## More Information

See `docs/cpp/README.md` for comprehensive documentation including:
- Detailed build instructions
- GitHub Pages setup
- Advanced configuration options
- Documentation writing best practices
