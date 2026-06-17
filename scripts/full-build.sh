#!/bin/bash

set -e
scripts/clean.sh
scripts/gen.sh
scripts/build.sh $@

# Also build all examples.
scripts/asm.sh examples/counter
scripts/asm.sh examples/timer
scripts/asm.sh examples/gb-minimal
scripts/asm.sh examples/gb-hello-world
scripts/asm.sh examples/gb-unbricked
scripts/asm.sh examples/gb-net-test
