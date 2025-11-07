#!/usr/bin/env bash
#
# Build script for Haiku OS
#
# This script builds DataPainter on Haiku OS with all dependencies.
# It handles dependency installation via pkgman, builds the project, and runs tests.
#
# Usage:
#   ./scripts/build-haiku.sh [--install-deps] [--clean] [--release|--debug] [--jobs N]
#
# Options:
#   --install-deps    Install dependencies via pkgman
#   --clean           Clean build directory before building
#   --release         Build in Release mode (default)
#   --debug           Build in Debug mode
#   --jobs N          Number of parallel build jobs (default: 4)
#   --help            Show this help message
#

set -e  # Exit on error
set -u  # Exit on undefined variable

# Default options
INSTALL_DEPS=false
CLEAN_BUILD=false
BUILD_TYPE="Release"
JOBS=4

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --install-deps)
            INSTALL_DEPS=true
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --jobs)
            JOBS="$2"
            shift 2
            ;;
        --help)
            head -n 20 "$0" | tail -n +2 | sed 's/^# //' | sed 's/^#//'
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Run with --help for usage information"
            exit 1
            ;;
    esac
done

# Check if we're actually on Haiku
if [[ "$(uname)" != "Haiku" ]]; then
    echo "Error: This script is for Haiku OS only"
    exit 1
fi

echo "=== DataPainter Haiku Build Script ==="
echo "Build type: $BUILD_TYPE"
echo "Parallel jobs: $JOBS"
echo

# Install dependencies if requested
if [ "$INSTALL_DEPS" = true ]; then
    echo "Installing dependencies via pkgman..."

    # Install build tools and libraries
    pkgman install -y \
        cmake \
        gcc \
        make \
        sqlite_devel \
        ncurses_devel \
        python3 \
        python3_pip

    echo "Dependencies installed successfully"
    echo
fi

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning build directory..."
    rm -rf build
    echo
fi

# Create build directory
echo "Creating build directory..."
mkdir -p build
cd build

# Configure CMake
echo "Configuring CMake..."
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
echo

# Build project
echo "Building DataPainter..."
cmake --build . --config "$BUILD_TYPE" -j "$JOBS"
echo

# Run unit tests
echo "Running unit tests..."
ctest --output-on-failure -j "$JOBS"
echo

echo "=== Build Complete ==="
echo "Binary location: build/datapainter"
echo
echo "To install system-wide, run:"
echo "  cd build && make install"
echo
echo "To run integration tests, run:"
echo "  cd .. && python3 -m pytest tests/integration/ -v"
echo
