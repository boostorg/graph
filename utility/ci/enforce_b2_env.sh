#! /bin/bash
#
# Copyright 2017 - 2019 James E. King III
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
#      http://www.boost.org/LICENSE_1_0.txt)
#
# Enforce B2 build variables understood by boost-ci scripts.
#

set -e

function enforce_b2
{
    local old_varname=$1
    local new_varname=B2_${old_varname}

    if [ -z "${!new_varname}" ]; then
        if [ ! -z "${!old_varname}" ]; then
            echo
            echo "WARNING: Your .travis.yml file needs to be updated:"
            echo "         use ${new_varname} instead of ${old_varname}"
            echo
            export ${new_varname}="${!old_varname}"
            unset ${old_varname}
        fi
    fi
}

enforce_b2 "CXXFLAGS"
enforce_b2 "CXXSTD"
enforce_b2 "DEFINES"
enforce_b2 "LINKFLAGS"
enforce_b2 "TESTFLAGS"
enforce_b2 "TOOLSET"

# default language level: C++11
if [ -z "$B2_CXXSTD" ]; then
    export B2_CXXSTD=11
fi

# default parallel build jobs: number of CPUs available + 1
if [ -z "${B2_JOBS}" ]; then
    cpus=$(cat /proc/cpuinfo | grep 'processor' | wc -l)
    export B2_JOBS=$((cpus + 1))
fi
