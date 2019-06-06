#!/bin/bash

# echo "using $TOOLSET : : $COMPILER ;" > ~/user-config.jam
# IFS=','
# for CXXLOCAL in $CXXSTD; do  (cd libs/config/test && ../../../b2 config_info_travis_install toolset=$TOOLSET cxxstd=$CXXLOCAL && ./config_info_travis && rm ./config_info_travis)  done
# unset IFS

set -ex

. $(dirname "${BASH_SOURCE[0]}")/../enforce_b2_env.sh

$BOOST_ROOT/b2 . toolset=$B2_TOOLSET cxxstd=$B2_CXXSTD $B2_CXXFLAGS $B2_DEFINES $B2_INCLUDE $B2_LINKFLAGS $B2_TESTFLAGS $B2_ADDRESS_MODEL $B2_LINK $B2_THREADING $B2_VARIANT -j${B2_JOBS} $* 