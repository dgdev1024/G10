#!/bin/bash

# Exit on any error
set -e

# Look for the `g10-boy` binary in either `./build/bin/linux-debug` or
# `./build/bin/linux-release`.
if [ -f "./build/bin/linux-debug-slow/g10-boy" ]; then
    BOY_BINARY="./build/bin/linux-debug-slow/g10-boy"
elif [ -f "./build/bin/linux-debug/g10-boy" ]; then
    BOY_BINARY="./build/bin/linux-debug/g10-boy"
elif [ -f "./build/bin/linux-release/g10-boy" ]; then
    BOY_BINARY="./build/bin/linux-release/g10-boy"
else
    echo "Error: g10-boy binary not found. Run './scripts/full-build.sh' to build it."
    exit 1
fi

"$BOY_BINARY" -r $@
