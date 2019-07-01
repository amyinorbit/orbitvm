//===--------------------------------------------------------------------------------------------===
// garbage.h - Orbit Garbage Collector API
// This source is part of Orbit
//
// Created on 2019-07-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_garbage_h
#define orbit_garbage_h

#include <orbit/common.h>
#include <orbit/rt2/value.h>

typedef struct sOrbitGC OrbitGC;

struct sOrbitGC {
    size_t allocated;
    size_t nextCollection;
    OrbitObject* head;
};

void orbit_gcInit(OrbitGC* self);
void orbit_gcDeinit(OrbitGC* self);

void orbit_gcRun(OrbitGC* self);

#endif