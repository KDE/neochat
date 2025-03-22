#!/usr/bin/env bash

set -e

export GIT_CLONE_ARGS="--depth 1 --single-branch"
export FLATPAK_DIR="$(readlink -f $(dirname $0))"
cd ${FLATPAK_DIR}

if [ ! -d flatpak-builder-tools ]; then
        git clone ${GIT_CLONE_ARGS} https://github.com/flatpak/flatpak-builder-tools
else
	git -C flatpak-builder-tools pull
fi

./flatpak-builder-tools/cargo/flatpak-cargo-generator.py -o generated-sources.json ../../integral/src/sdk/Cargo.lock
