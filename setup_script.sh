#!/bin/bash
# setup.sh - Setup script for Unix Shell Job Scheduler
# Usage: ./setup.sh

set -e  # Exit on error

echo "======================================"
echo "Unix Shell Job Scheduler - Setup"
echo "======================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored messages
print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_info() {
    echo -e "${YELLOW}ℹ${NC} $1"
}

# Check if GCC is installed
check_gcc() {
    print_info "Checking for GCC compiler..."
    if command -v gcc &> /dev/null; then
        GCC_VERSION=$(gcc --version | head -n1)
        print_success "GCC found: $GCC_VERSION"
        return 0
    else
        print_error "GCC not found!"
        echo "Please install GCC:"
        echo "  Ubuntu/Debian: sudo apt-get install build-essential"
        echo "  macOS: xcode-select --install"
        echo "  Fedora: sudo dnf install gcc"
        return 1
    fi
}

# Check if Make is installed
check_make() {
    print_info "Checking for Make..."
    if command -v make &> /dev/null; then
        MAKE_VERSION=$(make --version | head -n1)
        print_success "Make found: $MAKE_VERSION"
        return 0
    else
        print_error "Make not found!"
        echo "Please install Make:"
        echo "  Ubuntu/Debian: sudo apt-get install make"
        echo "  macOS: xcode-select --install"
        echo "  Fedora: sudo dnf install make"
        return 1
    fi
}

# Clean previous builds
clean_build() {
    print_info "Cleaning previous builds..."
    if [ -f "Makefile" ]; then
        make clean 2>/dev/null || true
        print_success "Cleaned build artifacts"
    fi
}

# Build the project
build_project() {
    print_info "Building project..."
    if make; then
        print_success "Build completed successfully!"
        return 0
    else
        print_error "Build failed!"
        return 1
    fi
}

# Verify binaries
verify_build() {
    print_info "Verifying build..."
    
    if [ -f "shell" ] && [ -x "shell" ]; then
        print_success "shell binary created"
    else
        print_error "shell binary not found or not executable"
        return 1
    fi
    
    if [ -f "test_program" ] && [ -x "test_program" ]; then
        print_success "test_program binary created"
    else
        print_error "test_program binary not found or not executable"
        return 1
    fi
    
    return 0
}

# Run basic tests
run_tests() {
    print_info "Running basic tests..."
    
    # Test 1: Shell starts and exits
    echo "exit" | ./shell > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        print_success "Test 1: Shell starts and exits correctly"
    else
        print_error "Test 1 failed"
        return 1
    fi
    
    # Test 2: Help command works
    echo -e "help\nexit" | ./shell > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        print_success "Test 2: Help command works"
    else
        print_error "Test 2 failed"
        return 1
    fi
    
    print_success "All basic tests passed!"
    return 0
}

# Create docs directory
setup_docs() {
    if [ ! -d "docs" ]; then
        print_info "Creating docs directory..."
        mkdir -p docs
        print_success "docs directory created"
    fi
}

# Main setup flow
main() {
    echo "Starting setup..."
    echo ""
    
    # Check prerequisites
    check_gcc || exit 1
    check_make || exit 1
    echo ""
    
    # Setup
    setup_docs
    clean_build
    echo ""
    
    # Build
    build_project || exit 1
    echo ""
    
    # Verify
    verify_build || exit 1
    echo ""
    
    # Test
    run_tests || print_error "Some tests failed (non-critical)"
    echo ""
    
    # Success message
    echo "======================================"
    print_success "Setup completed successfully!"
    echo "======================================"
    echo ""
    echo "Quick start:"
    echo "  ./shell              - Start the shell"
    echo "  ./test_program &     - Run test program in background"
    echo "  make clean           - Clean build files"
    echo "  make                 - Rebuild project"
    echo ""
    echo "Documentation:"
    echo "  README.md            - Project overview and usage"
    echo "  CONTRIBUTING.md      - Contribution guidelines"
    echo ""
    echo "Happy coding! 🚀"
}

# Run main function
main