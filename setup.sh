#!/usr/bin/env bash

top=$(dirname $0)

cd $top

mkdir -p build lib
cd lib
if [ ! -d googletest ]; then
  git clone https://github.com/google/googletest/
fi
