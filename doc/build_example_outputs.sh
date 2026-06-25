#!/usr/bin/env bash
# Compiles every .cpp under modules/ROOT/examples/ with -std=c++14, runs it,
# and writes stdout to a sibling .txt file that Antora pages can include.
#
# Usage:
#   BOOST_INCLUDE=/path/to/boost/include \
#   BOOST_LIB=/path/to/boost/lib       \
#   ./build_example_outputs.sh
#
# Defaults below match the Conan-installed Boost 1.79 layout used for
# local development. CI overrides them.

set -u

HERE=$(cd "$(dirname "$0")" && pwd)
REPO_ROOT=$(cd "$HERE/.." && pwd)
EXAMPLES_ROOT="$HERE/modules/ROOT/examples"

: "${BOOST_INCLUDE:=$HOME/.conan/data/boost/1.79.0/_/_/package/2c7a64ca03ffe00ac4600bf373ea3ecf338b36d9/include}"
: "${BOOST_LIB:=}"
: "${CXX:=g++}"
: "${CXXFLAGS:=-std=c++14 -O1 -w}"

if [ ! -d "$BOOST_INCLUDE" ]; then
    echo "BOOST_INCLUDE not found: $BOOST_INCLUDE" >&2
    echo "Set BOOST_INCLUDE to the Boost include directory and re-run." >&2
    exit 1
fi

# Per-example link flags. A handful of examples need the prebuilt
# boost_graph library (graphviz, graphml readers and the isomorphism
# example pull in non-header-only code).
extra_libs_for() {
    case "$1" in
        io/graphml.cpp|io/graphviz.cpp|algorithms/isomorphism/isomorphism.cpp)
            echo "-lboost_graph"
            ;;
        *)
            echo ""
            ;;
    esac
}

LDFLAGS=""
if [ -n "$BOOST_LIB" ]; then
    LDFLAGS="-L$BOOST_LIB -Wl,-rpath,$BOOST_LIB"
fi

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

ok=0
fail=0
fail_list=""

while IFS= read -r src; do
    rel=${src#"$EXAMPLES_ROOT/"}
    out_txt=${src%.cpp}.txt
    bin="$WORK/$(basename "$src" .cpp).out"
    extra=$(extra_libs_for "$rel")

    if ! "$CXX" $CXXFLAGS \
            -I"$REPO_ROOT/include" -I"$BOOST_INCLUDE" \
            "$src" -o "$bin" $LDFLAGS $extra 2>/dev/null; then
        printf '  [compile FAIL] %s\n' "$rel"
        fail=$((fail + 1))
        fail_list+="$rel (compile)\n"
        continue
    fi

    if ! "$bin" > "$out_txt.tmp" 2>&1; then
        printf '  [run FAIL]     %s\n' "$rel"
        rm -f "$out_txt.tmp"
        fail=$((fail + 1))
        fail_list+="$rel (run)\n"
        continue
    fi

    mv "$out_txt.tmp" "$out_txt"
    printf '  [OK]           %s\n' "$rel"
    ok=$((ok + 1))
done < <(find "$EXAMPLES_ROOT" -name '*.cpp' | sort)

echo
echo "Generated $ok .txt outputs, $fail failures."
if [ "$fail" -gt 0 ]; then
    printf "Failing examples:\n$fail_list"
    exit 1
fi
