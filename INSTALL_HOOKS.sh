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
HOOKS_SOURCE_DIR="$PROJECT_ROOT/scripts/hooks"

if [ ! -d "$HOOKS_DIR" ]; then
    echo "Error: Not in a git repository or .git/hooks directory not found"
    exit 1
fi

log_info "Project root: $PROJECT_ROOT"
log_info "Hooks directory: $HOOKS_DIR"
echo ""

# Step 1: Install hooks from scripts/hooks to .git/hooks
log_info "Installing git hooks from scripts/hooks/..."

if [ -d "$HOOKS_SOURCE_DIR" ]; then
    # Install pre-commit hook
    if [ -f "$HOOKS_SOURCE_DIR/pre-commit" ]; then
        cp "$HOOKS_SOURCE_DIR/pre-commit" "$HOOKS_DIR/pre-commit"
        chmod +x "$HOOKS_DIR/pre-commit"
        log_success "pre-commit hook installed"
    else
        log_warning "pre-commit hook template not found in $HOOKS_SOURCE_DIR"
    fi

    # Install pre-push hook
    if [ -f "$HOOKS_SOURCE_DIR/pre-push" ]; then
        cp "$HOOKS_SOURCE_DIR/pre-push" "$HOOKS_DIR/pre-push"
        chmod +x "$HOOKS_DIR/pre-push"
        log_success "pre-push hook installed"
    else
        log_warning "pre-push hook template not found in $HOOKS_SOURCE_DIR"
    fi

    # Install post-commit hook (optional)
    if [ -f "$HOOKS_SOURCE_DIR/post-commit" ]; then
        cp "$HOOKS_SOURCE_DIR/post-commit" "$HOOKS_DIR/post-commit"
        chmod +x "$HOOKS_DIR/post-commit"
        log_success "post-commit hook installed (background verification)"
    fi

    # Install post-push hook (optional)
    if [ -f "$HOOKS_SOURCE_DIR/post-push" ]; then
        cp "$HOOKS_SOURCE_DIR/post-push" "$HOOKS_DIR/post-push"
        chmod +x "$HOOKS_DIR/post-push"
        log_success "post-push hook installed (background build verification)"
    fi
else
    log_warning "Hooks source directory not found: $HOOKS_SOURCE_DIR"
    log_info "Hooks will not be installed from templates"
fi
echo ""

# Step 2: Verify hooks
log_info "Verifying installed hooks..."
if [ -x "$HOOKS_DIR/pre-commit" ]; then
    log_success "pre-commit hook verified"
else
    log_warning "pre-commit hook not executable or missing"
fi

if [ -x "$HOOKS_DIR/pre-push" ]; then
    log_success "pre-push hook verified"
else
    log_warning "pre-push hook not executable or missing"
fi

if [ -x "$HOOKS_DIR/post-commit" ]; then
    log_success "post-commit hook verified (optional)"
fi

if [ -x "$HOOKS_DIR/post-push" ]; then
    log_success "post-push hook verified (optional)"
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

# Step 5: Install pre-commit framework (recommended)
log_info "Setting up pre-commit framework..."

# Check for ~/.venv first
VENV_PRE_COMMIT="$HOME/.venv/bin/pre-commit"
if [ -x "$VENV_PRE_COMMIT" ]; then
    log_success "pre-commit found in ~/.venv"
    PRE_COMMIT_CMD="$VENV_PRE_COMMIT"
elif [ -x "$HOME/.venv/bin/python3" ]; then
    # Try installing to ~/.venv
    log_info "Installing pre-commit to ~/.venv..."
    "$HOME/.venv/bin/python3" -m pip install -q pre-commit 2>/dev/null || log_warning "Failed to install pre-commit to venv"
    if [ -x "$VENV_PRE_COMMIT" ]; then
        PRE_COMMIT_CMD="$VENV_PRE_COMMIT"
        log_success "pre-commit installed to ~/.venv"
    fi
fi

# Fallback to system pre-commit
if [ -z "$PRE_COMMIT_CMD" ]; then
    if command -v pre-commit &> /dev/null; then
        log_success "pre-commit framework found in system"
        PRE_COMMIT_CMD="pre-commit"
    else
        if command -v pip3 &> /dev/null; then
            log_info "Installing pre-commit framework..."
            pip3 install -q pre-commit || log_warning "Failed to install pre-commit"
            PRE_COMMIT_CMD="pre-commit"
        elif command -v pip &> /dev/null; then
            log_info "Installing pre-commit framework..."
            pip install -q pre-commit || log_warning "Failed to install pre-commit"
            PRE_COMMIT_CMD="pre-commit"
        fi
    fi
fi

# Install hooks using pre-commit framework
if [ -n "$PRE_COMMIT_CMD" ] && command -v "$PRE_COMMIT_CMD" &> /dev/null; then
    log_info "Installing pre-commit hooks..."
    if "$PRE_COMMIT_CMD" install --hook-stage pre-commit 2>/dev/null; then
        log_success "pre-commit hook installed"
    fi
    if "$PRE_COMMIT_CMD" install --hook-stage pre-push 2>/dev/null; then
        log_success "pre-push hook installed"
    fi
    if "$PRE_COMMIT_CMD" install --hook-stage post-commit 2>/dev/null; then
        log_success "post-commit hook installed"
    fi
    if "$PRE_COMMIT_CMD" install --hook-stage post-push 2>/dev/null; then
        log_success "post-push hook installed"
    fi
    
    # Also install hooks from scripts/hooks/ as backup
    log_info "Installing backup hooks from scripts/hooks/..."
    if [ -f "$HOOKS_SOURCE_DIR/pre-commit" ]; then
        cp "$HOOKS_SOURCE_DIR/pre-commit" "$HOOKS_DIR/pre-commit.backup"
        chmod +x "$HOOKS_DIR/pre-commit.backup"
    fi
    if [ -f "$HOOKS_SOURCE_DIR/pre-push" ]; then
        cp "$HOOKS_SOURCE_DIR/pre-push" "$HOOKS_DIR/pre-push.backup"
        chmod +x "$HOOKS_DIR/pre-push.backup"
    fi
    
    log_success "pre-commit framework is set up"
else
    log_warning "pre-commit framework not available"
    log_info "Hooks from scripts/hooks/ will be used instead"
fi
echo ""

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
