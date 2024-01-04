#!/bin/sh

script_dir=$(readlink -f $(dirname "$0"))
mkdir -p $script_dir/build/
cd $script_dir/build/
#export CC=clang
#export CXX=clang++
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4