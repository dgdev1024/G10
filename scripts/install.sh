#!/bin/bash
set -e

# Gen, then build release.
scripts/gen.sh
scripts/build.sh config=release

# Determine the install destination.
LIB_INSTALL_DIR="${1:-/usr/local/lib}"
BIN_INSTALL_DIR="${1:-/usr/local/bin}"

# Copy all libraries and executables to their appropriate destinations.
cp ./build/bin/linux-release/libG10.* "$LIB_INSTALL_DIR/"
cp ./build/bin/linux-release/g10-* "$BIN_INSTALL_DIR/"

echo "install ok"
