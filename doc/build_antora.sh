#!/bin/bash
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Run by boostorg/release-tools via the doc/Jamfile.v2 boostrelease target,
# following the boostorg/unordered scheme. Output lands in doc/html/.

set -ex

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$SCRIPT_DIR"

npm ci
npx antora graph-playbook.yml
cp -f html/index.html html/graph.html
