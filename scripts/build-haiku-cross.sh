#!/usr/bin/env bash
# Cross compile DataPainter for Haiku and create an hpkg package.
set -euo pipefail

# Get version from git tag or use 0.1.0 as default
VERSION=$(git describe --tags --abbrev=0 2>/dev/null | sed 's/^v//' || echo "0.1.0")
REVISION=1
ARCH=${HAIKU_ARCH:-x86_64}

########################################
# Assume cross compiler is already setup
########################################
CROSS_DIR=${CROSS_DIR:-$HOME/cross-tools-x86_64}
CROSS_BIN=${CROSS_BIN:-$CROSS_DIR/bin}
SYSROOT=${SYSROOT:-$CROSS_DIR/sysroot}
HOSTTOOLS_DIR=${HOSTTOOLS_DIR:-$HOME/haiku-hosttools}

if [ ! -d "$CROSS_BIN" ]; then
    for d in "$HOME"/cross-tools-*; do
        if [ -d "$d/bin" ]; then
            CROSS_BIN="$d/bin"
            CROSS_DIR="$d"
            break
        fi
    done
fi

if [ ! -d "$SYSROOT" ]; then
    for d in "$HOME"/cross-tools-*; do
        if [ -d "$d/sysroot" ]; then
            SYSROOT="$d/sysroot"
            break
        fi
    done
fi

if [ ! -d "$CROSS_BIN" ] || [ ! -d "$SYSROOT" ] || [ ! -d "$HOSTTOOLS_DIR" ]; then
    echo "Cross environment missing. Run setup-haiku-cross-env.sh first." >&2
    exit 1
fi

export PATH="$CROSS_BIN:$HOSTTOOLS_DIR:$PATH"
export LD_LIBRARY_PATH="$HOSTTOOLS_DIR:${LD_LIBRARY_PATH:-}"

########################################
# Check for required packages in sysroot
########################################

if [ ! -d "$SYSROOT/boot/system" ]; then
    echo "Haiku packages not found in $SYSROOT. Run setup-haiku-cross-env.sh first." >&2
    exit 1
fi

export CC=x86_64-unknown-haiku-gcc
export CXX=x86_64-unknown-haiku-g++
export PKG_CONFIG_PATH="$SYSROOT/boot/system/develop/lib/pkgconfig"
export CMAKE_PREFIX_PATH="$SYSROOT/boot/system/develop"

# Create a toolchain file for CMake
TOOLCHAIN_FILE=$(mktemp)
trap "rm -f $TOOLCHAIN_FILE" EXIT

cat > "$TOOLCHAIN_FILE" <<EOF
set(CMAKE_SYSTEM_NAME Haiku)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-unknown-haiku-gcc)
set(CMAKE_CXX_COMPILER x86_64-unknown-haiku-g++)

# Skip compiler tests that require linking (for cross-compilation)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH $SYSROOT/boot/system)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_SYSROOT $SYSROOT)

# Add library search paths for cross-compilation
list(APPEND CMAKE_LIBRARY_PATH
    "$SYSROOT/boot/system/develop/lib"
    "$SYSROOT/boot/system/lib"
)
list(APPEND CMAKE_INCLUDE_PATH
    "$SYSROOT/boot/system/develop/headers"
)
EOF

# Build with CMake
mkdir -p build-haiku
cd build-haiku
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$SYSROOT/boot/system/develop" \
    -DSQLite3_INCLUDE_DIR="$SYSROOT/boot/system/develop/headers" \
    -DSQLite3_LIBRARY="$SYSROOT/boot/system/develop/lib/x86_64/libsqlite3.a" \
    -DCURSES_INCLUDE_PATH="$SYSROOT/boot/system/develop/headers" \
    -DCURSES_LIBRARY="$SYSROOT/boot/system/develop/lib/x86_64/libncurses.a" \
    -DCURSES_NCURSES_LIBRARY="$SYSROOT/boot/system/develop/lib/x86_64/libncurses.a"
make -j"$(nproc)"
cd ..

# Create package directory structure
PKGDIR="build-haiku/hpkg/package-root"
rm -rf "$PKGDIR"
mkdir -p "$PKGDIR/bin"
cp build-haiku/datapainter "$PKGDIR/bin/"

# Determine where the Haiku host tools expect licenses to reside
LICENSE_DIR=$(strings "$HOSTTOOLS_DIR"/libpackage_build.so \
    | grep -o '/[^ ]*haiku/data/system/data' | head -n1)
if [ -z "$LICENSE_DIR" ]; then
    LICENSE_DIR="$(dirname "$HOSTTOOLS_DIR")/haiku/data/system/data"
fi
LICENSE_DIR="$LICENSE_DIR/licenses"
mkdir -p "$LICENSE_DIR"

# Create license file if it doesn't exist
if [ ! -f LICENSE ]; then
    echo "MIT License" > "$LICENSE_DIR/MIT"
else
    # Extract license name from LICENSE file or use MIT as default
    cp LICENSE "$LICENSE_DIR/MIT"
fi

PKGNAME="datapainter"
cat > "$PKGDIR/.PackageInfo" <<EOF2
name $PKGNAME
version ${VERSION}-$REVISION
architecture $ARCH
summary "TUI for creating two-dimensional labeled datasets"
description "DataPainter is an interactive terminal user interface (TUI) application for creating, editing, and visualizing two-dimensional labeled datasets stored in SQLite databases."
packager "DataPainter Developers <gregb@ifost.org.au>"
vendor "Industrial Linguistics"
licenses {
    "MIT"
}
copyrights {
    "2024 Greg Baker"
}
provides {
    $PKGNAME = ${VERSION}-$REVISION
    cmd:datapainter = ${VERSION}-$REVISION
}
requires {
    haiku
    lib:libsqlite3
    lib:libncurses
}
EOF2

package create -C "$PKGDIR" "${PKGNAME}_${VERSION}-${REVISION}_${ARCH}.hpkg"
