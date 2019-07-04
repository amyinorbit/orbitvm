//===--------------------------------------------------------------------------------------------===
// builder.h - Builder is the API used to emit Orbit bytecode
// This source is part of Orbit
//
// Created on 2019-07-04 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_builder_h
#define orbit_builder_h
#include <orbit/rt2/garbage.h>
#include <orbit/rt2/value.h>
#include <orbit/common.h>

struct sOrbitBuilder {
    OrbitGC* gc;
    OrbitFunction* function;
    OrbitASTContext* context;
};

#endif