#!/bin/bash

# echo "using $TOOLSET : : $COMPILER ;" > ~/user-config.jam
# IFS=','
# for CXXLOCAL in $CXXSTD; do  (cd libs/config/test && ../../../b2 config_info_travis_install toolset=$TOOLSET cxxstd=$CXXLOCAL && ./config_info_travis && rm ./config_info_travis)  done
# unset IFS

set -exv

. $(dirname "${BASH_SOURCE[0]}")/../enforce_b2_env.sh

cd $BOOST_ROOT/libs/$SELF

$BOOST_ROOT/b2 ./test/ toolset=$B2_TOOLSET cxxstd=$B2_CXXSTD cxxflags=\"$COPTS\" $B2_DEFINES $B2_INCLUDE linkflags=\"$LOPTS\" $B2_TESTFLAGS $B2_ADDRESS_MODEL $B2_LINK $B2_THREADING $B2_VARIANT -j${B2_JOBS} $* 
