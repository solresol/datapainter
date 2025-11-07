#!/usr/bin/env bash
#
# Build script for Windows (MinGW/Git Bash)
#
# This script builds DataPainter on Windows systems using MinGW or Git Bash.
# It handles dependency installation via vcpkg, builds the project, and runs tests.
#
# Usage (in Git Bash or MSYS2):
#   ./scripts/build-windows.sh [--install-deps] [--clean] [--release|--debug] [--jobs N]
#
# Options:
#   --install-deps    Install dependencies via vcpkg
#   --clean           Clean build directory before building
#   --release         Build in Release mode (default)
#   --debug           Build in Debug mode
#   --jobs N          Number of parallel build jobs (default: 4)
#   --help            Show this help message
#
# Prerequisites:
#   - Visual Studio 2017 or later (with C++ tools)
#   - CMake
#   - Git (for vcpkg)
#

set -e  # Exit on error
set -u  # Exit on undefined variable

# Default options
INSTALL_DEPS=false
CLEAN_BUILD=false
BUILD_TYPE="Release"
JOBS=4
VCPKG_ROOT="${VCPKG_ROOT:-C:/vcpkg}"

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
            head -n 25 "$0" | tail -n +2 | sed 's/^# //' | sed 's/^#//'
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Run with --help for usage information"
            exit 1
            ;;
    esac
done

echo "=== DataPainter Windows Build Script ==="
echo "Build type: $BUILD_TYPE"
echo "Parallel jobs: $JOBS"
echo "vcpkg root: $VCPKG_ROOT"
echo

# Install dependencies if requested
if [ "$INSTALL_DEPS" = true ]; then
    echo "Installing dependencies via vcpkg..."

    # Check if vcpkg is installed
    if [ ! -d "$VCPKG_ROOT" ]; then
        echo "vcpkg not found at $VCPKG_ROOT"
        echo "Installing vcpkg..."
        git clone https://github.com/Microsoft/vcpkg.git "$VCPKG_ROOT"
        cd "$VCPKG_ROOT"
        ./bootstrap-vcpkg.sh
        cd -
    fi

    # Install SQLite3
    "$VCPKG_ROOT/vcpkg" install sqlite3:x64-windows

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
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
echo

# Build project
echo "Building DataPainter..."
cmake --build . --config "$BUILD_TYPE" -j "$JOBS"
echo

# Run unit tests
echo "Running unit tests..."
ctest -C "$BUILD_TYPE" --output-on-failure -j "$JOBS"
echo

echo "=== Build Complete ==="
if [ "$BUILD_TYPE" = "Release" ]; then
    echo "Binary location: build/Release/datapainter.exe"
else
    echo "Binary location: build/Debug/datapainter.exe"
fi
echo
echo "Note: Windows TUI support is experimental"
echo
