#!/bin/bash

set -e
make -j2 $@
echo "build ok"
