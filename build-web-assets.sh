#!/bin/bash

# NOTE: Has to be changed depending on your own setup/path to emsdk
file_packager_dir=~/Documents/projects/emsdk/upstream/emscripten/tools/file_packager

build_dir="web-build"

$file_packager_dir $build_dir/assets.data --preload assets@assets --js-output=$build_dir/assets.js --use-preload-plugins
