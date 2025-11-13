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
    # Install necessary Haiku packages (sqlite3, ncurses)
    PKGS="sqlite ncurses6"
    BASE="https://eu.hpkg.haiku-os.org/haikuports/master/${ARCH}/current"

    if ! curl -sfI "$BASE/repo" >/dev/null; then
        echo "Unable to access HaikuPorts repository at $BASE" >&2
        exit 1
    fi

    mkdir -p "$SYSROOT/boot/system"
    curl -sSL "$BASE/repo" -o repo.hpkg
    if [ ! -s repo.hpkg ]; then
        echo "Failed to download repository index from $BASE" >&2
        exit 1
    fi
    package_repo list -f repo.hpkg | sed 's/^[[:space:]]*//' > repo.txt

    for p in $PKGS; do
        FILE=$(grep -E "^${p}-.*-${ARCH}\.hpkg$" repo.txt | sort -V | tail -1)
        if [ ! -f "$FILE" ]; then
            echo "Fetching $FILE"
            curl -sSL -o "$FILE" "$BASE/packages/$FILE"
        fi
        package extract -C "$SYSROOT/boot/system" "$FILE"
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
