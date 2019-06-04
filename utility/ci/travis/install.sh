#!/bin/bash

set -ex

if [ "$TRAVIS_OS_NAME" == "osx" ]; then
    unset -f cd
fi


${TRAVIS_BUILD_DIR}/utility/ci/enforce_b2_env.sh

function show_bootstrap_log
{
    cat bootstrap.log
}

export SELF=`basename $TRAVIS_BUILD_DIR`
cd ..

if [ "$TRAVIS_BRANCH" == "master" ]; then
    export BOOST_BRANCH="master"
else
    export BOOST_BRANCH="develop"
fi




git clone -b $BOOST_BRANCH --depth 1 https://github.com/boostorg/boost.git boost-root
cd boost-root

git submodule update --init
git submodule update --init libs/headers
git submodule update --init tools/boost_install
git submodule update --init tools/boostdep
git submodule update --init tools/build
git submodule update --init libs/config
cp -r $TRAVIS_BUILD_DIR/* libs/$SELF

export BOOST_ROOT="`pwd`"
export PATH="`pwd`":$PATH

python tools/boostdep/depinst/depinst.py $SELF


# If clang was installed from LLVM APT it will not have a /usr/bin/clang++
# so we need to add the correctly versioned llvm bin path to the PATH
if [ "${B2_TOOLSET%%-*}" == "clang" ]; then
    ver="${B2_TOOLSET#*-}"
    export PATH=/usr/lib/llvm-${ver}/bin:$PATH
    ls -ls /usr/lib/llvm-${ver}/bin || true
    hash -r || true
    which clang || true
    which clang++ || true

    # Additionally, if B2_TOOLSET is clang variant but CXX is set to g++
    # (it is on Travis CI) then boost build silently ignores B2_TOOLSET and
    # uses CXX instead
    if [ "${CXX}" != "clang"* ]; then
        echo "CXX is set to ${CXX} in this environment which would override"
        echo "the setting of B2_TOOLSET=clang, therefore we clear CXX here."
        export CXX=
    fi
fi


./bootstrap.sh
if [[ "${B2_TOOLSET}" == "gcc-"* ]] || [[ "${B2_TOOLSET}" == "clang-"* ]]; then
    ./bootstrap.sh --with-toolset=${B2_TOOLSET%%-*}
else
    ./bootstrap.sh --with-toolset=${B2_TOOLSET}
fi

./b2 headers

if [[ ! -z $CXXSTD ]] ; then
	cd "$BOOST_ROOT"
	echo "using $TOOLSET : : $COMPILER ;" > ~/user-config.jam
	IFS=','
	for CXXLOCAL in $CXXSTD; do  (cd libs/config/test && ../../../b2 config_info_travis_install toolset=$TOOLSET cxxstd=$CXXLOCAL && ./config_info_travis && rm ./config_info_travis)  done
	unset IFS

	./b2 -j3 libs/$SELF/test toolset=$TOOLSET cxxstd=$CXXSTD
fi