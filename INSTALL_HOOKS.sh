#!/bin/bash
################################################################################
# ASTERIX Git Hooks Installation Script
#
# This script sets up git hooks for the ASTERIX project.
# Usage: bash INSTALL_HOOKS.sh [--full]
#
# Options:
#   --full    Install pre-commit framework and all dependencies
#   --help    Show this help message
#
################################################################################

set -e

# Color codes
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[OK]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

show_help() {
    cat << EOF
ASTERIX Git Hooks Installation Script

Usage: bash INSTALL_HOOKS.sh [OPTIONS]

Options:
    --full      Install pre-commit framework and optional dependencies
    --help      Show this help message

Examples:
    # Basic setup (recommended)
    bash INSTALL_HOOKS.sh

    # Full setup with all tools
    bash INSTALL_HOOKS.sh --full

EOF
}

# Parse arguments
FULL_SETUP=false
while [[ $# -gt 0 ]]; do
    case $1 in
        --full)
            FULL_SETUP=true
            shift
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

echo ""
echo "=========================================="
echo "  ASTERIX Git Hooks Installation"
echo "=========================================="
echo ""

# Get project root
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
HOOKS_DIR="$PROJECT_ROOT/.git/hooks"

if [ ! -d "$HOOKS_DIR" ]; then
    echo "Error: Not in a git repository or .git/hooks directory not found"
    exit 1
fi

log_info "Project root: $PROJECT_ROOT"
log_info "Hooks directory: $HOOKS_DIR"
echo ""

# Step 1: Make hooks executable
log_info "Making hooks executable..."
if [ -f "$HOOKS_DIR/pre-commit" ]; then
    chmod +x "$HOOKS_DIR/pre-commit"
    log_success "pre-commit hook is now executable"
else
    log_warning "pre-commit hook not found"
fi

if [ -f "$HOOKS_DIR/pre-push" ]; then
    chmod +x "$HOOKS_DIR/pre-push"
    log_success "pre-push hook is now executable"
else
    log_warning "pre-push hook not found"
fi
echo ""

# Step 2: Verify hooks
log_info "Verifying hooks..."
if [ -x "$HOOKS_DIR/pre-commit" ]; then
    log_success "pre-commit hook verified"
else
    log_warning "pre-commit hook not executable"
fi

if [ -x "$HOOKS_DIR/pre-push" ]; then
    log_success "pre-push hook verified"
else
    log_warning "pre-push hook not executable"
fi
echo ""

# Step 3: Install basic Python tools
log_info "Installing essential Python tools..."

if command -v pip3 &> /dev/null; then
    echo "Installing: black, flake8, pytest, pytest-cov, coverage"
    pip3 install -q black flake8 pytest pytest-cov coverage || log_warning "Some Python packages failed to install"
    log_success "Python tools installed"
elif command -v pip &> /dev/null; then
    echo "Installing: black, flake8, pytest, pytest-cov, coverage"
    pip install -q black flake8 pytest pytest-cov coverage || log_warning "Some Python packages failed to install"
    log_success "Python tools installed"
else
    log_warning "pip not found, skipping Python tools installation"
fi
echo ""

# Step 4: Detect OS and install system tools
log_info "Detecting operating system..."

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="Ubuntu/Debian"
    log_info "Detected: $OS"

    if command -v apt-get &> /dev/null; then
        log_warning "System tools require sudo. Run the following to install:"
        echo ""
        echo "sudo apt-get install -y clang-format clang-tools valgrind cppcheck"
        echo ""
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macOS"
    log_info "Detected: $OS"

    if command -v brew &> /dev/null; then
        log_warning "System tools require Homebrew. Run the following to install:"
        echo ""
        echo "brew install clang-format valgrind cppcheck"
        echo ""
    fi
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    OS="Windows"
    log_info "Detected: $OS"
    log_warning "For Windows, use Git Bash or WSL and follow Ubuntu/Debian instructions"
fi
echo ""

# Step 5: Optional pre-commit framework setup
if [ "$FULL_SETUP" = true ]; then
    log_info "Setting up pre-commit framework..."

    if command -v pre-commit &> /dev/null; then
        log_success "pre-commit framework already installed"
    else
        if command -v pip3 &> /dev/null; then
            echo "Installing pre-commit framework..."
            pip3 install -q pre-commit || log_warning "Failed to install pre-commit"
        elif command -v pip &> /dev/null; then
            echo "Installing pre-commit framework..."
            pip install -q pre-commit || log_warning "Failed to install pre-commit"
        fi
    fi

    if command -v pre-commit &> /dev/null; then
        echo "Installing pre-commit hooks..."
        pre-commit install --install-hooks 2>/dev/null || log_warning "Failed to install pre-commit hooks"
        log_success "pre-commit framework is set up"
    else
        log_warning "pre-commit framework not available, skipping"
    fi
    echo ""
fi

# Step 6: Summary
echo "=========================================="
echo "Installation Summary"
echo "=========================================="
echo ""

log_success "Basic setup complete!"
echo ""
echo "Hooks are now ready to use. Next commit/push will trigger them automatically."
echo ""
echo "To test the hooks:"
echo "  1. Make a test change: echo '# test' >> README.md"
echo "  2. Stage the change: git add README.md"
echo "  3. Commit: git commit -m 'test: verify hooks work'"
echo ""
echo "If hooks don't execute:"
echo "  chmod +x .git/hooks/pre-commit .git/hooks/pre-push"
echo ""
echo "For comprehensive documentation, see:"
echo "  - GIT_HOOKS_SETUP.md (setup and troubleshooting)"
echo "  - CONTRIBUTING.md (development guidelines)"
echo "  - GIT_HOOKS_IMPLEMENTATION_SUMMARY.md (implementation details)"
echo ""

if [ "$FULL_SETUP" = false ]; then
    echo "For full setup with all optional tools and pre-commit framework:"
    echo "  bash INSTALL_HOOKS.sh --full"
    echo ""
fi

echo "=========================================="
