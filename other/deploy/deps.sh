#!/usr/bin/env bash

set -eux -o pipefail

if [ -d "prefix" ]; then
  exit 0
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

git clone --depth=1 https://github.com/TokTok/dockerfiles "$SCRIPT_DIR/dockerfiles"

SYSTEM="$1"
ARCH="$2"

DEP_PREFIX="$PWD/prefix"

for dep in sodium opus vpx; do
  mkdir -p "external/$dep"
  pushd "external/$dep"
  if [ -f "$SCRIPT_DIR/dockerfiles/qtox/build_${dep}_$SYSTEM.sh" ]; then
    SCRIPT="$SCRIPT_DIR/dockerfiles/qtox/build_${dep}_$SYSTEM.sh"
  else
    SCRIPT="$SCRIPT_DIR/dockerfiles/qtox/build_$dep.sh"
  fi
  "$SCRIPT" --arch "$SYSTEM-$ARCH" --libtype "static" --buildtype "release" --prefix "$DEP_PREFIX" --macos "10.15"
  popd
  rm -rf "external/$dep"
done
