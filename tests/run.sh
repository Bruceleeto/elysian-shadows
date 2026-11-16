#!/bin/sh
set -eu

test_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
test_build="$test_root/build/sol2-tests"
mkdir -p "$test_build"
cd "$test_root"

${CXX:-c++} -std=c++17 -fno-exceptions -fno-rtti -Wall -Wextra -Werror \
    -DSOL_ALL_SAFETIES_ON=1 -DSOL_NO_CHECK_NUMBER_PRECISION=1 \
    -IAssetIO/include -Isol2/include $(pkg-config --cflags lua5.5) \
    tests/binding.cpp $(pkg-config --libs lua5.5) \
    -o "$test_build/binding"
"$test_build/binding"

${CXX:-c++} -std=c++17 -Wall -Wextra -Werror \
    $(pkg-config --cflags lua5.5) tests/check_scripts.cpp \
    $(pkg-config --libs lua5.5) -o "$test_build/check_scripts"

${CXX:-c++} -std=c++17 -fno-exceptions -fno-rtti -Wall -Wextra -Werror -fPIC \
    -DSOL_ALL_SAFETIES_ON=1 -DSOL_NO_CHECK_NUMBER_PRECISION=1 \
    -IAssetIO/include -IQESTkUtils/include -Isol2/include $(pkg-config --cflags lua5.5 Qt6Core) \
    tests/qt_values.cpp QESTkUtils/source/tk_lua_qt_utils.cpp \
    $(pkg-config --libs lua5.5 Qt6Core) -o "$test_build/qt_values"
"$test_build/qt_values"

if [ "$#" -gt 0 ]; then
    "$test_build/check_scripts" "$@"
fi
