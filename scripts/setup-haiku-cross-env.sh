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
    set -uo pipefail  # Don't use -e, we handle errors explicitly

    HAIKU_BRANCH=${HAIKU_BRANCH:-r1beta5}   # use 'master' if you want nightly
    HAIKU_BASE="https://eu.hpkg.haiku-os.org/haiku/${HAIKU_BRANCH}/${ARCH}/current"
    HAIKUPORTS_BASE="https://eu.hpkg.haiku-os.org/haikuports/master/${ARCH}/current/packages"

    mkdir -p "$SYSROOT/boot/system"

    # Discover the current Haiku version string (e.g. r1~beta5_hrev57937_129)
    echo "Querying Haiku repo for current version..."
    route=$(curl -fsSL "$HAIKU_BASE")
    version=$(printf "%s\n" "$route" | sed -n 's/.*version: "\([^"]*\)".*/\1/p')
    if [ -z "${version:-}" ]; then
        echo "oops: couldn't extract Haiku version from $HAIKU_BASE" >&2
        exit 1
    fi
    echo "Found Haiku version: $version"

    # Fetch and extract core OS packages from the Haiku repo
    for pkg in haiku haiku_devel; do
        url="${HAIKU_BASE}/packages/${pkg}-${version}-1-${ARCH}.hpkg"
        filename="${pkg}-${version}-1-${ARCH}.hpkg"
        echo "Downloading ${url}…"

        # First try: check what redirect we get with verbose output
        echo "Testing redirect behavior..."
        curl -vsSLI --max-time 30 "$url" 2>&1 | head -20 || true

        # Add retry and be explicit about following redirects
        if ! curl -fsSL --retry 3 --retry-delay 2 --max-time 60 -o "$filename" "$url"; then
            echo "Failed to download $filename, trying CDN directly..." >&2
            # Fallback: try the CDN URL directly
            cdn_url="https://haiku-repository.cdn.haiku-os.org/${HAIKU_BRANCH}/${ARCH}/${version}/packages/${filename}"
            echo "Attempting CDN URL: ${cdn_url}"
            curl -vsSL --max-time 60 -o "$filename" "$cdn_url" 2>&1 | head -20 || {
                echo "oops: failed to download $filename from both URLs" >&2
                exit 1
            }
        fi
        "$HOSTTOOLS_DIR/package" extract -C "$SYSROOT/boot/system" "$filename"
        sleep 2  # Longer pause between downloads to avoid rate limiting
    done

    # Helper to get "latest" port package by name from HaikuPorts
    fetch_port_pkg() {
        local name="$1"
        # Grab directory index and pick the highest version for our arch
        local latest
        latest=$(curl -fsSL "$HAIKUPORTS_BASE/" \
            | grep -Eo "${name}-[^\"']+-${ARCH}\\.hpkg" \
            | sort -V | tail -1)
        if [ -z "${latest:-}" ]; then
            echo "oops: couldn't find ${name} in HaikuPorts at $HAIKUPORTS_BASE" >&2
            return 1
        fi
        echo "Downloading ${HAIKUPORTS_BASE}/${latest}…"
        curl -fsSLO "${HAIKUPORTS_BASE}/${latest}"
        "$HOSTTOOLS_DIR/package" extract -C "$SYSROOT/boot/system" "$latest"
    }

    # Ports needed for datapainter build
    fetch_port_pkg sqlite_devel
    fetch_port_pkg ncurses6_devel

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
