#!/bin/bash
set -e

# Build macOS .pkg installer
# Usage: ./scripts/build-macos-pkg.sh <version>

if [ -z "$1" ]; then
    echo "Usage: $0 <version>" >&2
    echo "Example: $0 0.1.0" >&2
    exit 1
fi

VERSION="$1"
BINARY="${2:-build/datapainter}"
MAN_PAGE="${3:-docs/datapainter.1}"

if [ ! -f "$BINARY" ]; then
    echo "Error: Binary not found at $BINARY" >&2
    exit 1
fi

if [ ! -f "$MAN_PAGE" ]; then
    echo "Error: Man page not found at $MAN_PAGE" >&2
    exit 1
fi

echo "Building macOS .pkg installer for version $VERSION..."

# Create staging directory structure
PKG_ROOT="pkg-root"
rm -rf "$PKG_ROOT"
mkdir -p "$PKG_ROOT/usr/local/bin"
mkdir -p "$PKG_ROOT/usr/local/share/man/man1"

# Copy files to staging
echo "Copying binary and man page..."
cp "$BINARY" "$PKG_ROOT/usr/local/bin/"
cp "$MAN_PAGE" "$PKG_ROOT/usr/local/share/man/man1/"

# Build the .pkg
PKG_NAME="datapainter-${VERSION}.pkg"
echo "Creating $PKG_NAME..."
pkgbuild --root "$PKG_ROOT" \
         --identifier com.industrial-linguistics.datapainter \
         --version "$VERSION" \
         --install-location / \
         "$PKG_NAME"

echo "Package created successfully: $PKG_NAME"
echo "Contents:"
pkgutil --payload-files "$PKG_NAME"

# Clean up staging directory
rm -rf "$PKG_ROOT"

echo ""
echo "To install: sudo installer -pkg $PKG_NAME -target /"
