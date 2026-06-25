#!/bin/bash

set -e
make -j8 $@
echo "build ok"
