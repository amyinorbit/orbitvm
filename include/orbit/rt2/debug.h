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

#include <orbit/rt2/chunk.h>

void orbit_debugChunk(const OrbitChunk* chunk, const char* name);
int orbit_debugInstruction(const OrbitChunk* chunk, int offset);

#endif