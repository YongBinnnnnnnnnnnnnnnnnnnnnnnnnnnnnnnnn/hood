#!/bin/sh
cd "$(dirname "$0")"
mkdir -p libs/$(uname -s)
cd libs/

if [ -z $HOOD_PROXY_BUILD_CONCURRENCY ]; then
  HOOD_PROXY_BUILD_CONCURRENCY=$(nproc --all)
fi

host_arch=$(uname -r|cut -d - -f 3)
arch=$host_arch
toolset=gcc


for arg in "$@"; do
  case $arg in 
    arch=*) arch=$(echo $arg|sed "s/[^=]*=//");;
  esac
done

sudo apt install -y cmake build-essential clang clang-tidy clang-format

if [ "$host_arch" != "$arch" ]; then
  if [ "$arch" = "armhf" ]; then
    sudo apt install -y gcc-arm-linux-gnueabihf
    toolset=gcc-arm-linux-gnueabihf
  fi
  if [ "$arch" = "arm64" ]; then
    sudo apt install -y gcc-aarch64-linux-gnu 
    toolest=gcc-aarch64-linux-gnu 
  fi
fi

export TAR_OPTIONS=--no-same-owner

if [ -d ./$(uname -s)/$arch/openssl/include ] ; then
  echo "openssl already installed."
else
  echo "Downloading openssl."
  openssl_src=$(curl -s https://www.openssl.org/source/|grep "tar.gz</a"|tail -n 1|sed -r -e "s/.*gz\">([^<]*).*/\1/")
  curl -L -O -C - https://www.openssl.org/source/$openssl_src
  echo "Extracting openssl."
  tar -xmf $openssl_src
  openssl_src=$(echo $openssl_src |sed "s/.......$//")
  cd $openssl_src
  install_dir=../$(uname -s)/$arch/openssl
  mkdir -p $install_dir
  install_dir=$(realpath $install_dir)
  ./Configure no-ssl3 no-shared no-module no-weak-ssl-ciphers no-unit-test no-apps no-quic no-uplink --prefix=$install_dir
  make -j ${HOOD_PROXY_BUILD_CONCURRENCY}
  make install_sw
  cd ..
fi

# ---- begin code copied and modified from carla Simulator which is MIT License
# ==============================================================================
# -- Get boost includes --------------------------------------------------------
# ==============================================================================

BOOST_VERSION="1.84.0"
BOOST_FILE_NAME=$(echo boost_$BOOST_VERSION|tr '.' '_')

if [ -d ./$(uname -s)/$arch/boost/lib ] ; then
  echo "${BOOST_FILE_NAME} already installed."
else
  if [ ! -d "${BOOST_FILE_NAME}-source" ]; then
    if [ ! -f ${BOOST_FILE_NAME}".tar.gz" ] ; then
      echo "Downloading boost."
      curl -L -O -C - "https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION}/source/${BOOST_FILE_NAME}.tar.gz"
    fi
    echo "Extracting boost."
    tar -xmf $BOOST_FILE_NAME.tar.gz --exclude "libs" --exclude "doc" --exclude "example" --exclude "test"
    tar -xmf $BOOST_FILE_NAME.tar.gz --exclude "test" --exclude "example" --exclude "doc" ${BOOST_FILE_NAME}/libs/
    mkdir -p $(uname -s)/$arch/boost/include
  fi
  cd ${BOOST_FILE_NAME}

  if [ ! -f "b2" ] ; then
    ./bootstrap.sh --prefix="../$(uname -s)/$arch/boost"
  fi

  BOOST_CXXFLAGS="-std=c++20 -I /$(uname -s)/$arch/openssl/include"
  
  ./b2 toolset=$toolset link=static cxxflags="${BOOST_CXXFLAGS}" --without-chrono --without-graph --without-graph_parallel --without-mpi --without-python --without-stacktrace -j ${HOOD_PROXY_BUILD_CONCURRENCY} release install

  # Get rid of  python2 build artifacts completely & do a clean build for python3
  cd -

fi

unset BOOST_FILE_NAME
# ---- end code copied and modified from carla Simulator which is MIT License
