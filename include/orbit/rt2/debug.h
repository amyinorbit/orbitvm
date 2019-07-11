//===--------------------------------------------------------------------------------------------===
// debug.h - Support API for runtime/vm debugging
// This source is part of Orbit
//
// Created on 2019-06-27 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_debug
#define orbit_debug

#include <orbit/rt2/value.h>

void orbitDebugFunction(const OrbitFunction* fn, const char* name);
int orbitDebugInstruction(const OrbitFunction* fn, int offset);

#endif