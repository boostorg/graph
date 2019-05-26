#!/bin/bash
#
# Copyright 2018 - 2019 James E. King III
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
#      http://www.boost.org/LICENSE_1_0.txt)
#
# Bash script to run in travis to perform a cppcheck
# cwd should be $BOOST_ROOT before running
#

set -ex

# valgrind on travis (xenial) is 3.11 which is old
# using valgrind 3.14 but we have to build it

pushd /tmp
git clone -b VALGRIND_3_14_0 git://sourceware.org/git/valgrind.git
cd valgrind

./autogen.sh
./configure --prefix=/tmp/vg
make -j3
make -j3 install
popd

export PATH=/tmp/vg/bin:$PATH
export B2_INCLUDE=include=/tmp/vg/include

ci/travis/build.sh
