#!/bin/sh

#export PREFIX=/usr
#export TRAVIS_BUILD_DIR=/root
export PREFIX=$HOME/xorg
export PKG_CONFIG_PATH=$PREFIX/pkgconfig:$PREFIX/share/pkgconfig:$PKG_CONFIG_PATH
export PIGLIT_DIR=$TRAVIS_BUILD_DIR/piglit
export XTEST_DIR=$TRAVIS_BUILD_DIR/xtest

set -e

meson -Dprefix=$PREFIX build/
ninja -C build/ install
ninja -C build/ test
