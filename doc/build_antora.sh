#!/bin/bash
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/boostorg/graph
#
# Picked up by boostorg/release-tools' build_docs/{linuxdocs.sh,macosdocs.sh,
# windowsdocs.ps1} via auto-detection: presence of this file selects the
# Antora build pipeline. Output lands in doc/build/site/.

set -xe

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$SCRIPT_DIR"

if [ $# -eq 0 ]; then
  PLAYBOOK="playbook.yml"
else
  PLAYBOOK="$1"
fi

echo "Installing npm dependencies..."
npm ci

echo "Building docs with Antora using $PLAYBOOK..."
PATH="$(pwd)/node_modules/.bin:${PATH}"
export PATH
npx antora --clean --fetch "$PLAYBOOK"
echo "Done"
