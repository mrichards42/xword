#!/bin/bash
# Script to build and install LuaJIT.

set -e

LUAJIT_VERSION=$1

if [[ -z "$LUAJIT_VERSION" ]]; then
  echo "First argument must be set to the LuaJIT version"
  exit 1
fi

INSTALL_PATH="$HOME/LuaJIT-$LUAJIT_VERSION"
mkdir -p $INSTALL_PATH

echo "Building LuaJIT version $LUAJIT_VERSION"

if [[ "$OSTYPE" == "darwin"* ]]; then
  BUILD_COMMAND="make MACOSX_DEPLOYMENT_TARGET=10.7 CC='gcc -arch x86_64'"
  INSTALL_COMMAND="cp libluajit.a $INSTALL_PATH/lib/liblua51.a"
  LIBS_DIR="macosx"
elif [[ "$OSTYPE" == "linux"* ]]; then
  BUILD_COMMAND="make CC='gcc -fPIC'"
  INSTALL_COMMAND="cp libluajit.a $INSTALL_PATH/lib/liblua51.a"
  LIBS_DIR="linux"
else  # Windows
  BUILD_COMMAND="./msvcbuild.bat"
  INSTALL_COMMAND="cp lua51.{dll,exp,lib} $INSTALL_PATH/lib/"
  LIBS_DIR="windows"
fi

# Check to see if the cache directory is empty
if [ ! -d "$INSTALL_PATH/lib" ]; then
  curl -fsSL -o LuaJIT-$LUAJIT_VERSION.tar.gz https://luajit.org/download/LuaJIT-2.0.5.tar.gz
  tar -xzf LuaJIT-$LUAJIT_VERSION.tar.gz
  cd LuaJIT-$LUAJIT_VERSION/src
  eval $BUILD_COMMAND
  mkdir -p $INSTALL_PATH/lib
  eval $INSTALL_COMMAND
  cd ../../
else
  echo "Using cached directory."
fi

mkdir -p deps/luajit/lib/$LIBS_DIR/
cp -a $INSTALL_PATH/lib/* deps/luajit/lib/$LIBS_DIR/
