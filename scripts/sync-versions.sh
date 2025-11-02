#!/bin/bash
# Script to synchronize versions from VERSION file to component-specific files
# This ensures all components use versions from the single source of truth

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
VERSION_FILE="$PROJECT_ROOT/VERSION"

if [ ! -f "$VERSION_FILE" ]; then
    echo "Error: VERSION file not found at $VERSION_FILE" >&2
    exit 1
fi

# Read versions from VERSION file
extract_version() {
    local key="$1"
    grep -E "^${key}[ \t]*=" "$VERSION_FILE" | sed -E 's/^[^=]+=[ \t]*//' | head -n 1 | tr -d '[:space:]'
}

CPP_VERSION=$(extract_version "CPP_VERSION" || extract_version "PROJECT_VERSION")
PYTHON_VERSION=$(extract_version "PYTHON_VERSION")
RUST_VERSION=$(extract_version "RUST_VERSION")

if [ -z "$CPP_VERSION" ]; then
    echo "Error: Could not extract CPP_VERSION or PROJECT_VERSION from VERSION file" >&2
    exit 1
fi

echo "Synchronizing versions:"
echo "  C++:    $CPP_VERSION"
echo "  Python: $PYTHON_VERSION"
echo "  Rust:   $RUST_VERSION"

# Update Rust Cargo.toml
if [ -f "$PROJECT_ROOT/asterix-rs/Cargo.toml" ] && [ -n "$RUST_VERSION" ]; then
    echo "Updating asterix-rs/Cargo.toml..."
    sed -i "s/^version = \".*\"/version = \"$RUST_VERSION\"/" "$PROJECT_ROOT/asterix-rs/Cargo.toml"
    echo "  ✓ Updated Rust version to $RUST_VERSION"
fi

# Update Python version.py (if it exists and needs to be updated)
if [ -f "$PROJECT_ROOT/asterix/version.py" ] && [ -n "$PYTHON_VERSION" ]; then
    echo "Updating asterix/version.py..."
    echo "__version__ = '$PYTHON_VERSION'" > "$PROJECT_ROOT/asterix/version.py"
    echo "  ✓ Updated Python version to $PYTHON_VERSION"
fi

echo "Version synchronization complete!"

