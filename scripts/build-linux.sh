#!/usr/bin/env bash
#
# Build script for Linux (Ubuntu/Debian/Fedora)
#
# This script builds DataPainter on Linux systems with all dependencies.
# It handles dependency installation, builds the project, and runs tests.
#
# Usage:
#   ./scripts/build-linux.sh [--install-deps] [--clean] [--release|--debug] [--jobs N]
#
# Options:
#   --install-deps    Install system dependencies (requires sudo)
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

# Detect Linux distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    DISTRO=$ID
else
    echo "Cannot detect Linux distribution"
    exit 1
fi

echo "=== DataPainter Linux Build Script ==="
echo "Distribution: $DISTRO"
echo "Build type: $BUILD_TYPE"
echo "Parallel jobs: $JOBS"
echo

# Install dependencies if requested
if [ "$INSTALL_DEPS" = true ]; then
    echo "Installing dependencies..."
    case $DISTRO in
        ubuntu|debian)
            sudo apt-get update
            sudo apt-get install -y \
                build-essential \
                cmake \
                libsqlite3-dev \
                libncurses-dev \
                python3 \
                python3-pip
            ;;
        fedora|rhel|centos)
            sudo dnf install -y \
                gcc-c++ \
                cmake \
                sqlite-devel \
                ncurses-devel \
                python3 \
                python3-pip
            ;;
        arch|manjaro)
            sudo pacman -S --noconfirm \
                base-devel \
                cmake \
                sqlite \
                ncurses \
                python \
                python-pip
            ;;
        *)
            echo "Unsupported distribution: $DISTRO"
            echo "Please install dependencies manually:"
            echo "  - C++ compiler (g++ or clang)"
            echo "  - CMake 3.10+"
            echo "  - SQLite3 development libraries"
            echo "  - ncurses development libraries"
            echo "  - Python 3.7+"
            exit 1
            ;;
    esac
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
echo "  cd build && sudo make install"
echo
echo "To run integration tests, run:"
echo "  cd .. && uv run pytest tests/integration/ -v"
echo
