//===--------------------------------------------------------------------------------------------===
// value_array.h - Mutable array object used by the runtime and exposed to Orbit.
// This source is part of Orbit
//
// Created on 2019-06-27 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_value_array_h
#define orbit_value_array_h

#include <orbit/rt2/value.h>

typedef struct sOrbitArray OrbitArray;

struct sOrbitArray {
    size_t count;
    size_t capacity;
    OrbitValue* data;
};

void orbit_arrayInit(OrbitArray* self);
void orbit_arrayDeinit(OrbitArray* self);
void orbit_arrayAppend(OrbitArray* self, OrbitValue item);

#endif