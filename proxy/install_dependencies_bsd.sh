#!/bin/sh
cd "$(dirname "$0")"
mkdir -p libs/$(uname -s)
cd libs/

if [ -z $HOOD_PROXY_BUILD_CONCURRENCY ]; then
  HOOD_PROXY_BUILD_CONCURRENCY=2
fi

# ---- begin code copied and modified from carla Simulator which is MIT License
# ==============================================================================
# -- Get boost includes --------------------------------------------------------
# ==============================================================================

BOOST_VERSION="1.83.0"
BOOST_DOWNLOAD_NAME=$(echo $BOOST_VERSION|tr '.' '_')

if [ -d "${BOOST_BASENAME}-install" ] ; then
  echo "${BOOST_BASENAME} already installed."
else
  if [ ! -d "${BOOST_BASENAME}-source" ]; then
    if [ ! -f "boost_"${BOOST_DOWNLOAD_NAME}".tar.gz" ] ; then
      echo "Downloading boost."
      curl -L -O -C - "https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION}/source/boost_${BOOST_DOWNLOAD_NAME}.tar.gz"
    fi
    echo "Extracting boost."
    tar -xvmf boost_1_83_0.tar.gz --exclude "boost" --exclude "libs" --exclude "doc" --exclude "example" --exclude "test"
    tar -xvmf boost_1_83_0.tar.gz --exclude "test" --exclude "example" --exclude "doc" boost_1_83_0/more boost_1_83_0/tools boost_1_83_0/status
    mkdir -p $(uname -s)/${BOOST_BASENAME}-install/include
    mv ${BOOST_DOWNLOAD_NAME} ${BOOST_DOWNLOAD_NAME}-source
  fi
  cd ${BOOST_BASENAME}-source

  if [ ! -f "b2" ] ; then
    ./bootstrap.sh --prefix="../$(uname -s)/${BOOST_BASENAME}-install"
  fi

  BOOST_CXXFLAGS="-I . -std=c++20"

  ./b2 cxxflags="${BOOST_CXXFLAGS}" -j ${HOOD_PROXY_BUILD_CONCURRENCY} stage release
  ./b2 -j ${HOOD_PROXY_BUILD_CONCURRENCY} install
  ./b2 -j ${HOOD_PROXY_BUILD_CONCURRENCY} --clean-all

  # Get rid of  python2 build artifacts completely & do a clean build for python3
  cd -

fi

unset BOOST_BASENAME
# ---- end code copied and modified from carla Simulator which is MIT License
