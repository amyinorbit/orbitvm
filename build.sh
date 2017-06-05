#!/usr/bin/env bash
# Orbit Cmake build driver

mkdir -p build
cd build
cmake ..
cmake --build . --target install

