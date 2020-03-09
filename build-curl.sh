#!/bin/bash
# Script to build and install Curl.

set -e

CURL_VERSION=$1

if [[ -z "$CURL_VERSION" ]]; then
  echo "First argument must be set to the curl version"
  exit 1
fi

INSTALL_PATH="$HOME/curl-$CURL_VERSION"
mkdir -p $INSTALL_PATH

echo "Building curl version $CURL_VERSION"

# Check to see if the cache directory is empty
if [ ! -d "$INSTALL_PATH/lib" ]; then
  curl -fsSL -o curl-$CURL_VERSION.tar.bz2 https://curl.haxx.se/download/curl-$CURL_VERSION.tar.bz2
  tar -xjf curl-$CURL_VERSION.tar.bz2
  cd curl-$CURL_VERSION/winbuild
  nmake.exe -f Makefile.vc mode=dll VC=12
  cp -a ../builds/libcurl-vc12-x86-release-dll-ipv6-sspi-winssl/* $INSTALL_PATH/
  cd ../../
else
  echo "Using cached directory."
fi

mkdir -p deps/curl/
cp -a $INSTALL_PATH/include/ deps/curl/
mkdir -p deps/curl/lib/
cp $INSTALL_PATH/bin/libcurl.dll deps/curl/lib/
cp $INSTALL_PATH/lib/libcurl.lib deps/curl/lib/
