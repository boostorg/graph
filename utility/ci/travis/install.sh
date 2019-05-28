#!/bin/bash

cd $BOOST_ROOT/libs/graph
BOOST_BRANCH=develop && [ "$TRAVIS_BRANCH" == "master" ] && BOOST_BRANCH=master || true
cd ..
git clone -b $BOOST_BRANCH --depth 1 https://github.com/boostorg/boost.git boost-root
cd boost-root
git submodule update --init tools/boost_install
git submodule update --init libs/headers
git submodule update --init tools/build
git submodule update --init libs/config
git submodule update --init tools/boostdep
cp -r $TRAVIS_BUILD_DIR/* libs/graph
python tools/boostdep/depinst/depinst.py graph
./bootstrap.sh
./b2 headers