#!/bin/sh

script_dir=$(readlink -f $(dirname "$0"))
#export CC=clang
#export CXX=clang++

type=release
arch=$(uname -r|cut -d - -f 3)
system=$(uname -s)

for arg in "$@"; do
  case $arg in 
    type=*) type=$(echo $arg|sed "s/[^=]*=//");;
    arch=*) arch=$(echo $arg|sed "s/[^=]*=//");;
  esac
done

mkdir -p $script_dir/build/$type/$arch
cd $script_dir/build/$type/$arch

cmake -DCMAKE_BUILD_TYPE=$type -DOPENSSL_ROOT_DIR=$script_dir/libs/$system/$arch/openssl -DBOOST_ROOT=$script_dir/libs/$system/$arch/boost $script_dir
make -j $(nproc --all)
strip hood_proxy -o hood_proxy_s
sha256sum hood_proxy hood_proxy_s
