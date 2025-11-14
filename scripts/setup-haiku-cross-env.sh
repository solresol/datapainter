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
    PKG_CACHE="${HAIKU_PKG_CACHE:-}"  # Optional cache directory from environment

    mkdir -p "$SYSROOT/boot/system"
    if [ -n "$PKG_CACHE" ]; then
        mkdir -p "$PKG_CACHE"
        echo "Using package cache: $PKG_CACHE"
    fi

    # Discover the current Haiku version string (e.g. r1~beta5_hrev57937_129)
    echo "Querying Haiku repo for current version..."
    route=$(curl -fsSL "$HAIKU_BASE")
    version=$(printf "%s\n" "$route" | sed -n 's/.*version: "\([^"]*\)".*/\1/p')
    if [ -z "${version:-}" ]; then
        echo "oops: couldn't extract Haiku version from $HAIKU_BASE" >&2
        exit 1
    fi
    echo "Found Haiku version: $version"

    # Fetch and extract core OS packages
    # Download from GitHub releases to avoid Haiku CDN rate limiting
    # See: https://github.com/solresol/datapainter/releases/tag/haiku-deps-r1beta5
    GITHUB_RELEASE="https://github.com/solresol/datapainter/releases/download/haiku-deps-r1beta5"

    for pkg in haiku haiku_devel; do
        filename="${pkg}-${version}-1-${ARCH}.hpkg"

        # Check if package is in cache
        if [ -n "$PKG_CACHE" ] && [ -f "$PKG_CACHE/$filename" ]; then
            echo "Using cached $filename"
            cp "$PKG_CACHE/$filename" "$filename"
        else
            echo "Downloading ${pkg} from GitHub releases..."
            # GitHub converts ~ to . in uploaded filenames, so we need to do the same for the URL
            github_filename=$(printf "%s" "$filename" | sed 's/~/./')
            url="${GITHUB_RELEASE}/${github_filename}"

            curl -fsSL --retry 3 --retry-delay 2 --max-time 120 \
                -o "$filename" "$url" || {
                echo "oops: failed to download $filename from GitHub releases" >&2
                echo "URL: $url" >&2
                exit 1
            }

            # Save to cache for future use
            if [ -n "$PKG_CACHE" ]; then
                cp "$filename" "$PKG_CACHE/$filename"
                echo "Cached $filename for future builds"
            fi
        fi

        "$HOSTTOOLS_DIR/package" extract -C "$SYSROOT/boot/system" "$filename"
    done

    # Download HaikuPorts packages from GitHub releases (to avoid CDN rate limiting)
    # These packages are pinned versions hosted in the haiku-deps-r1beta5 release
    for pkg_filename in "sqlite_devel-3.47.2.0-1-x86_64.hpkg" "ncurses6_devel-6.5-3-x86_64.hpkg"; do
        # Check if package is in cache
        if [ -n "$PKG_CACHE" ] && [ -f "$PKG_CACHE/$pkg_filename" ]; then
            echo "Using cached $pkg_filename"
            cp "$PKG_CACHE/$pkg_filename" "$pkg_filename"
        else
            echo "Downloading $pkg_filename from GitHub releases..."
            url="${GITHUB_RELEASE}/${pkg_filename}"

            curl -fsSL --retry 3 --retry-delay 2 --max-time 120 \
                -o "$pkg_filename" "$url" || {
                echo "oops: failed to download $pkg_filename from GitHub releases" >&2
                echo "URL: $url" >&2
                exit 1
            }

            # Save to cache for future use
            if [ -n "$PKG_CACHE" ]; then
                cp "$pkg_filename" "$PKG_CACHE/$pkg_filename"
                echo "Cached $pkg_filename for future builds"
            fi
        fi

        "$HOSTTOOLS_DIR/package" extract -C "$SYSROOT/boot/system" "$pkg_filename"
    done

    # Download gcc_syslibs_devel from HaikuPorts (not in GitHub release)
    # This package contains libstdc++ which is needed for linking
    echo "Downloading gcc_syslibs_devel from HaikuPorts..."
    # Query the repo for available gcc_syslibs_devel packages
    gcc_pkg=$(curl -fsSL "${HAIKUPORTS_BASE}/" | grep -o 'gcc_syslibs_devel-[^"]*-x86_64\.hpkg' | head -1)

    if [ -z "$gcc_pkg" ]; then
        echo "oops: could not find gcc_syslibs_devel package in HaikuPorts" >&2
        exit 1
    fi

    # Check if package is in cache
    if [ -n "$PKG_CACHE" ] && [ -f "$PKG_CACHE/$gcc_pkg" ]; then
        echo "Using cached $gcc_pkg"
        cp "$PKG_CACHE/$gcc_pkg" "$gcc_pkg"
    else
        echo "Downloading $gcc_pkg from HaikuPorts..."
        curl -fsSL --retry 3 --retry-delay 2 --max-time 120 \
            -o "$gcc_pkg" "${HAIKUPORTS_BASE}/${gcc_pkg}" || {
            echo "oops: failed to download $gcc_pkg from HaikuPorts" >&2
            exit 1
        }

        # Save to cache for future use
        if [ -n "$PKG_CACHE" ]; then
            cp "$gcc_pkg" "$PKG_CACHE/$gcc_pkg"
            echo "Cached $gcc_pkg for future builds"
        fi
    fi

    "$HOSTTOOLS_DIR/package" extract -C "$SYSROOT/boot/system" "$gcc_pkg"

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
