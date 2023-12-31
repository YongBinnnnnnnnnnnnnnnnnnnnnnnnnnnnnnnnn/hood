#!/bin/sh
cd "$(dirname "$0")"
mkdir -p libs/$(uname -s)
cd libs/

if [ -z $HOOD_PROXY_BUILD_CONCURRENCY ]; then
  HOOD_PROXY_BUILD_CONCURRENCY=4
fi

sudo apt install -y cmake build-essential clang clang-tidy clang-format libssl-dev

export TAR_OPTIONS=--no-same-owner

# ---- begin code copied and modified from carla Simulator which is MIT License
# ==============================================================================
# -- Get boost includes --------------------------------------------------------
# ==============================================================================

BOOST_VERSION="1.84.0"
BOOST_FILE_NAME=$(echo boost_$BOOST_VERSION|tr '.' '_')

if [ -d "${BOOST_FILE_NAME}-install" ] ; then
  echo "${BOOST_FILE_NAME} already installed."
else
  if [ ! -d "${BOOST_FILE_NAME}-source" ]; then
    if [ ! -f ${BOOST_FILE_NAME}".tar.gz" ] ; then
      echo "Downloading boost."
      curl -L -O -C - "https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION}/source/${BOOST_FILE_NAME}.tar.gz"
    fi
    echo "Extracting boost."
    tar -xvmf $BOOST_FILE_NAME.tar.gz --exclude "libs" --exclude "doc" --exclude "example" --exclude "test"
    tar -xvmf $BOOST_FILE_NAME.tar.gz --exclude "test" --exclude "example" --exclude "doc" ${BOOST_FILE_NAME}/libs/
    mkdir -p $(uname -s)/${BOOST_FILE_NAME}-install/include
    mv ${BOOST_FILE_NAME} ${BOOST_FILE_NAME}-source
  fi
  cd ${BOOST_FILE_NAME}-source

  if [ ! -f "b2" ] ; then
    ./bootstrap.sh --prefix="../$(uname -s)/${BOOST_FILE_NAME}-install"
  fi

  BOOST_CXXFLAGS="-std=c++20"

  ./b2 link=static cxxflags="${BOOST_CXXFLAGS}" -j ${HOOD_PROXY_BUILD_CONCURRENCY} stage release install

  # Get rid of  python2 build artifacts completely & do a clean build for python3
  cd -

fi

unset BOOST_FILE_NAME
# ---- end code copied and modified from carla Simulator which is MIT License
