#!/bin/bash

set -e
make -j$(nproc) $@
echo "build ok"
