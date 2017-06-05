#!/usr/bin/env bash
# Orbit Cmake build driver

mkdir -p build
cd build
cmake .. -G Ninja > /dev/null
cmake --build . --target clean
cmake --build . --target all
ctest .

