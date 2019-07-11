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

#define ORBIT_GC_MAXROOTS (256)

struct sOrbitGC {
    size_t allocated;
    size_t nextCollection;
    OrbitObject* head;
    
    size_t rootCount;
    OrbitObject* roots[ORBIT_GC_MAXROOTS];
};

void orbitGCInit(OrbitGC* self);
void orbitGCDeinit(OrbitGC* self);

void orbitGCRun(OrbitGC* self);

void orbitGCPush(OrbitGC* self, OrbitObject* root);
void orbitGCPop(OrbitGC* self);
void orbitGCRelease(OrbitGC* self, OrbitObject* root);

#endif