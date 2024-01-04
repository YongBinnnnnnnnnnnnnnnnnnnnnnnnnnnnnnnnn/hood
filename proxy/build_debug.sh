#!/bin/sh

script_dir=$(readlink -f $(dirname "$0"))
mkdir -p $script_dir/build/debug
cd $script_dir/build/debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
make -j4