#!/bin/bash
# Script to build and install wxWidgets, with our custom tweaks patch.

set -e

WX_VERSION=$1
CONFIGURATION=$2

if [[ -z "$WX_VERSION" ]]; then
  echo "First argument must be set to the wxWindows version"
  exit 1
fi

INSTALL_PATH="$HOME/wxWidgets-$WX_VERSION"
mkdir -p $INSTALL_PATH

WX_CONFIGURE_FLAGS="\
  --disable-shared \
  --enable-compat28 \
  --with-macosx-version-min=10.7 \
  CFLAGS=-fvisibility-inlines-hidden \
  CXXFLAGS='-fvisibility-inlines-hidden -stdlib=libc++' \
  CPPFLAGS='-fvisibility-inlines-hidden -stdlib=libc++' \
  OBJCFLAGS=-fvisibility-inlines-hidden \
  OBJCXXFLAGS='-fvisibility-inlines-hidden -stdlib=libc++' \
  LDFLAGS=-stdlib=libc++"
WX_MAKE_FLAGS="SHARED=0"

if [[ "$CONFIGURATION" == "Debug" ]]; then
  WX_CONFIGURE_FLAGS="$WX_CONFIGURE_FLAGS --enable-debug"
  WX_MAKE_FLAGS="$WX_MAKE_FLAGS BUILD=debug"
elif [[ "$CONFIGURATION" == "Release" ]]; then
  WX_MAKE_FLAGS="$WX_MAKE_FLAGS BUILD=release"
else
  echo "Unsupported configuration: $CONFIGURATION"
  echo "Please use one of Debug or Release"
  exit 1
fi
echo "Building wxWidgets version $WX_VERSION for configuration: $CONFIGURATION"
mkdir -p $INSTALL_PATH

if [[ "$OSTYPE" == "darwin"* ]]; then
  # Mac OS X
  BUILD_COMMAND="./configure --prefix=$INSTALL_PATH $WX_CONFIGURE_FLAGS && make -j 4 && make install"
else
  # Windows
  BUILD_COMMAND="sed -e 's/WXWIN_COMPATIBILITY_2_8 0/WXWIN_COMPATIBILITY_2_8 1/' -i include/wx/msw/setup.h"
  BUILD_COMMAND="$BUILD_COMMAND && cd build/msw && nmake.exe -f makefile.vc $WX_MAKE_FLAGS"
  BUILD_COMMAND="$BUILD_COMMAND && cp -a ../../lib ../../include $INSTALL_PATH"
fi

# Check to see if the cache directory is empty
if [ ! -d "$INSTALL_PATH/lib" ]; then
  curl -fsSL -o wxWidgets-$WX_VERSION.tar.bz2 https://github.com/wxWidgets/wxWidgets/releases/download/v$WX_VERSION/wxWidgets-$WX_VERSION.tar.bz2
  tar -xjf wxWidgets-$WX_VERSION.tar.bz2
  cd wxWidgets-$WX_VERSION
  patch -p1 -i ../wxaui-tweaks.patch
  patch -p1 -i ../wxWidgets-osx-private-build-fix.patch
  eval $BUILD_COMMAND
else
  echo "Using cached directory."
fi
