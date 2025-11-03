#!/usr/bin/env bash
set -euo pipefail

apt-get update
apt-get install -y cmake g++ libsqlite3-dev libncurses-dev

(
  cd "$(dirname "$0")/tests/integration"
  uv sync
)
