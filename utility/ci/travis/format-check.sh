#!/bin/bash -ue

set -o errexit
set -o pipefail
set -o nounset
set -ev

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then

  find ${TRAVIS_BUILD_DIR} -type f -name '*.h' -o -name '*.cpp' \
  | xargs -I{} -P "$(nproc)" clang-format -i -style=file {}

  dirty=$(git ls-files --modified)

  if [[ $dirty ]]; then
    echo "The following files have been modified:"
    echo "$dirty"

    exit 1
  fi
fi