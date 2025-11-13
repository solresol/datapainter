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
    # Install necessary Haiku packages (haiku, haiku_devel, sqlite_devel, ncurses6_devel)
    # Using _devel packages to get headers and libraries for cross-compilation

    mkdir -p "$SYSROOT/boot/system"

    # Query the Haiku repo for the current stable version
    HAIKU_REPO="https://eu.hpkg.haiku-os.org/haiku/r1beta5/${ARCH}/current"
    echo "Querying Haiku repo for current version..."
    VERSION=$(curl -sL "$HAIKU_REPO" | grep -o 'r1~beta[0-9]*_hrev[0-9]*_[0-9]*' | head -1)

    if [ -z "$VERSION" ]; then
        echo "Error: Could not determine current Haiku version" >&2
        exit 1
    fi
    echo "Found Haiku version: $VERSION"

    # Download haiku and haiku_devel from Haiku repo (core OS packages)
    HAIKU_PKG_BASE="$HAIKU_REPO/packages"
    for pkg in "haiku" "haiku_devel"; do
        FILE="${pkg}-${VERSION}-1-${ARCH}.hpkg"
        echo "Downloading $FILE from Haiku repo..."
        if curl -sLf -o "$FILE" "$HAIKU_PKG_BASE/$FILE"; then
            echo "Extracting $FILE to sysroot..."
            "$HOSTTOOLS_DIR/package" extract -C "$SYSROOT/boot/system" "$FILE"
        else
            echo "Error: Failed to download $FILE" >&2
            exit 1
        fi
    done

    # Download ports (sqlite_devel, ncurses6_devel) from HaikuPorts
    PORTS_BASE="https://eu.hpkg.haiku-os.org/haikuports/master/${ARCH}/current/packages"
    for port in "sqlite_devel" "ncurses6_devel"; do
        echo "Finding latest $port in HaikuPorts..."
        # Scrape directory listing for latest version
        FILE=$(curl -sL "$PORTS_BASE/" | grep -o "href=\"${port}-[^\"]*-${ARCH}\.hpkg\"" | sed 's/href="//;s/"//' | sort -V | tail -1)

        if [ -z "$FILE" ]; then
            echo "Error: Could not find $port package" >&2
            exit 1
        fi

        echo "Downloading $FILE from HaikuPorts..."
        if curl -sLf -o "$FILE" "$PORTS_BASE/$FILE"; then
            echo "Extracting $FILE to sysroot..."
            "$HOSTTOOLS_DIR/package" extract -C "$SYSROOT/boot/system" "$FILE"
        else
            echo "Error: Failed to download $FILE" >&2
            exit 1
        fi
    done

    echo "All packages installed successfully to sysroot"
}

if [ ! -d "$CROSS_BIN" ] || [ ! -d "$HOSTTOOLS_DIR" ]; then
    fetch_tools
fi
export PATH="$CROSS_BIN:$HOSTTOOLS_DIR:$PATH"
export LD_LIBRARY_PATH="$HOSTTOOLS_DIR:${LD_LIBRARY_PATH:-}"

if [ ! -d "$SYSROOT/boot/system" ]; then
    install_haiku_packages
fi
