#! /bin/bash
#
# Copyright 2018 - 2019 James E. King III
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
#      http://www.boost.org/LICENSE_1_0.txt)
#
# Bash script to run in travis to perform a cppcheck
# cwd should be $BOOST_ROOT before running
#

set -exv

. $(dirname "${BASH_SOURCE[0]}")/../enforce_b2_env.sh || exit 1


STANDARDS=
while IFS=',' read -ra ADDR; do
    for i in "${ADDR[@]}"; do
        # process "$i"
        STANDARDS="${STANDARDS} --std=c++${i}"
    done
done <<< "$B2_CXXSTD"

cppcheck -I. ${STANDARDS} --enable=all --error-exitcode=1 \
     --force --check-config . || exit 1
