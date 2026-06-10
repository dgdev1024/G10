#!/bin/bash

set -e
scripts/clean.sh
scripts/gen.sh
scripts/build.sh $@

# Also build all examples.
for example in examples/*; do
    if [ -d "$example" ]; then
        scripts/asm.sh $example
    fi
done
