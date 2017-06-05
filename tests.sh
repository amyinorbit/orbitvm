#!/usr/bin/env bash
# Orbit Cmake build driver

mkdir -p build
cd build
cmake .. -G Ninja > /dev/null
cmake --build . --target clean > /dev/null
cmake --build . --target all > /dev/null
ctest .

