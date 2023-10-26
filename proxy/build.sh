#!/bin/sh

script_dir=$(readlink -f $(dirname "$0"))
mkdir -p $script_dir/build/
cd $script_dir/build/
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja ..
ninja
CXX=clang++
CFLAGS=" -std=c++20 -I /usr/local/include/ -Wall -v"
$CXX ../src/main.cpp -o hood_proxy