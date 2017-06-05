#!/usr/bin/env bash
# Orbit Cmake build driver

mkdir -p build
cd build
cmake .. -G Ninja
cmake --build . --target install

