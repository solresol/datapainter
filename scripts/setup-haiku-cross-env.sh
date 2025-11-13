#!/usr/bin/env bash
# Setup environment for Haiku cross compilation (datapainter).
set -euo pipefail

ARCH=${HAIKU_ARCH:-x86_64}
CROSS_DIR=${CROSS_DIR:-$HOME/cross-tools-${ARCH}}
CROSS_BIN=${CROSS_BIN:-$CROSS_DIR/bin}
SYSROOT=${SYSROOT:-$CROSS_DIR/sysroot}
HOSTTOOLS_DIR=${HOSTTOOLS_DIR:-$HOME/haiku-hosttools}

fetch_tools() {
    echo "Fetching Haiku cross compiler..."
    sudo apt-get update
    sudo apt-get install -y jq unzip cmake
    if [ ! -d toolchain ]
    then
        git clone --depth=1 https://github.com/haiku/haiku-toolchains-ubuntu.git toolchain
    fi
    echo "Cloned"
    pushd toolchain >/dev/null
    echo "pushed"
    hosttools_url=$(./fetch.sh --hosttools)
    curl -sLJO "$hosttools_url"
    echo "Curled once"
    buildtools_url=$(./fetch.sh --buildtools --arch="$ARCH")
    curl -sLJO "$buildtools_url"
    echo "Curled a second time"
    unzip -qo ${ARCH}-linux-hosttools-*.zip -d "$HOSTTOOLS_DIR"
    echo "Unzipped"
    unzip -qo ${ARCH}-linux-buildtools-*.zip -d "$HOME"
    echo "Unzipped some more"
    popd >/dev/null
    echo "Popped"
}

install_haiku_packages() {
    # Install necessary Haiku packages (haiku system, sqlite_devel, ncurses6_devel)
    # Using _devel packages to get headers and libraries for cross-compilation
    BASE="https://eu.hpkg.haiku-os.org/haikuports/master/${ARCH}/current/packages"

    mkdir -p "$SYSROOT/boot/system"

    # Hardcoded package versions (update periodically as needed)
    # curl follows redirects to CDN automatically
    PACKAGES=(
        "haiku-r1~beta5_hrev57937+129-1-x86_64.hpkg"
        "sqlite_devel-3.47.0-1-x86_64.hpkg"
        "ncurses6_devel-6.5-1-x86_64.hpkg"
    )

    for PKG in "${PACKAGES[@]}"; do
        echo "Downloading $PKG..."
        if curl -sLf -o "$PKG" "$BASE/$PKG"; then
            echo "Extracting $PKG to sysroot..."
            "$HOSTTOOLS_DIR/package" extract -C "$SYSROOT/boot/system" "$PKG"
        else
            echo "Warning: Failed to download $PKG" >&2
        fi
    done
}

if [ ! -d "$CROSS_BIN" ] || [ ! -d "$HOSTTOOLS_DIR" ]; then
    fetch_tools
fi
export PATH="$CROSS_BIN:$HOSTTOOLS_DIR:$PATH"
export LD_LIBRARY_PATH="$HOSTTOOLS_DIR:${LD_LIBRARY_PATH:-}"

if [ ! -d "$SYSROOT/boot/system" ]; then
    install_haiku_packages
fi
