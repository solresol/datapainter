#!/usr/bin/env bash
# Update a simple Haiku repository with the built .hpkg package
set -e

REPO_DIR="${1:-repo}"

# Ensure the Haiku host tools are available. They are installed by
# `setup-haiku-cross-env.sh` under $HOME/haiku-hosttools. If the PATH was not
# propagated from that script (e.g. in a fresh shell) try adding the directory
# here before failing.
if ! command -v package_repo >/dev/null 2>&1; then
    if [ -d "$HOME/haiku-hosttools" ]; then
        export PATH="$HOME/haiku-hosttools:$PATH"
        export LD_LIBRARY_PATH="$HOME/haiku-hosttools:${LD_LIBRARY_PATH:-}"
    fi
fi

if ! command -v package_repo >/dev/null 2>&1; then
    echo "package_repo command not found. Run setup-haiku-cross-env.sh first." >&2
    exit 1
fi

mkdir -p "$REPO_DIR/packages"
cp datapainter_*.hpkg "$REPO_DIR/packages/"

cat > "$REPO_DIR/repo.info" <<EOF
name DataPainter
vendor "Industrial Linguistics"
summary "DataPainter repository"
priority 1
baseurl https://packages.industrial-linguistics.com/datapainter/haiku/repo
identifier tag:industrial-linguistics.com,2024:datapainter
architecture x86_64
EOF

package_repo create "$REPO_DIR/repo.info" "$REPO_DIR"/packages/*.hpkg
