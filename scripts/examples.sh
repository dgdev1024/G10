#!/bin/bash
set -e

scripts/gen.sh
scripts/build.sh $@

scripts/asm.sh examples/counter
scripts/asm.sh examples/timer
scripts/asm.sh examples/gb-minimal
scripts/asm.sh examples/gb-hello-world
scripts/asm.sh examples/gb-unbricked