#!/bin/sh
# Запускать из каталога библиотеки (тестовые заглушки не входят в src/).
set -eu
cd "$(dirname "$0")/.."
CXX="${CXX:-g++}"
EXE="${TMPDIR:-/tmp}/flprog_additional_test_$$"
trap 'rm -f "$EXE"' EXIT HUP INT TERM
"$CXX" -std=c++11 -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -Itests/stubs -Isrc \
  tests/test_aditional.cpp src/batteryTester.cpp src/sharicAuto.cpp -o "$EXE"
"$EXE"
echo 'FLPROG_ADITIONALLIBS_TESTS_PASS'
