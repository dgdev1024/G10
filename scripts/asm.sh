#!/bin/bash

# Exit on any error
set -e

# Look for the `g10-asm` binary in either `./build/bin/linux-debug` or
# `./build/bin/linux-release`.
if [ -f "./build/bin/linux-debug-slow/g10-asm" ]; then
    ASM_BINARY="./build/bin/linux-debug-slow/g10-asm"
elif [ -f "./build/bin/linux-debug/g10-asm" ]; then
    ASM_BINARY="./build/bin/linux-debug/g10-asm"
elif [ -f "./build/bin/linux-release/g10-asm" ]; then
    ASM_BINARY="./build/bin/linux-release/g10-asm"
else
    echo "Error: g10-asm binary not found. Run './scripts/full-build.sh' to build it."
    exit 1
fi

# Catalogue all `.asm` files in the provided directory (first argument)
# e.g. `./examples/counter`. For each, run the assembler.
ASM_FILES=$(find "$1" -name "*.asm" -type f)
ASM_OUTPUT_FOLDER="./build/$1"
mkdir -p "$ASM_OUTPUT_FOLDER"
for ASM_FILE in $ASM_FILES; do
    ASM_OUTPUT_FILE="$ASM_OUTPUT_FOLDER/$(basename "${ASM_FILE%.asm}.o")"
    "$ASM_BINARY" -i "$ASM_FILE" -o "$ASM_OUTPUT_FILE" $@
done

# Link all object files
OBJECT_FILES=$(find "$ASM_OUTPUT_FOLDER" -name "*.o" -type f)
"$ASM_BINARY" -L $OBJECT_FILES -o "./build/bin/$(basename "$1").g10"
