#!/bin/bash

cd "$BOOST_ROOT"
echo "using $TOOLSET : : $COMPILER ;" > ~/user-config.jam
IFS=','
for CXXLOCAL in $CXXSTD; do  (cd libs/config/test && ../../../b2 config_info_travis_install toolset=$TOOLSET cxxstd=$CXXLOCAL && ./config_info_travis && rm ./config_info_travis)  done
unset IFS
./b2 -j3 libs/$SELF/test toolset=$TOOLSET cxxstd=$CXXSTD