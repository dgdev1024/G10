#!/bin/bash

set -e
tools/premake5 gmake $@
echo "gen ok"
