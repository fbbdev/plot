#!/bin/sh

# WARNING: ONLY FOR TRAVIS CI! DO NOT RUN

set -e

mkdir -p "$HOME/deps"

if [ ! -d "$HOME/deps/cmake/bin" ]; then
  wget --no-check-certificate -O cmake.tar.gz "https://cmake.org/files/v3.4/cmake-3.4.0-Linux-x86_64.tar.gz"
  tar -xf cmake.tar.gz
  mv cmake-* "$HOME/deps/cmake"
  rm -rf cmake.tar.gz cmake-*
fi
