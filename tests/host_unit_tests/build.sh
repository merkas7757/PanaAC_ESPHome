#!/usr/bin/env bash
# Build and run the panaac host protocol unit tests.
# Compiles the UNMODIFIED component (panaac.cpp + extra.cpp) against minimal ESPHome
# stubs placed first on the include path, links a doctest-free test binary, runs it.
set -eu
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
COMP="$REPO/esphome/components/panaac"
STUBS="$HERE/stubs"
CXX="${CXX:-g++}"

echo ">> compiling"
"$CXX" -std=gnu++20 -O0 -g \
  -I "$STUBS" \
  -I "$COMP" \
  "$HERE/test_main.cpp" "$COMP/panaac.cpp" "$COMP/extra.cpp" \
  -o "$HERE/test_host" 2> "$HERE/.build.log" || { cat "$HERE/.build.log"; exit 1; }

echo ">> running"
"$HERE/test_host"