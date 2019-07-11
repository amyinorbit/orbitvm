//===--------------------------------------------------------------------------------------------===
// map.h - Runtime hashmap-backed associative container
// This source is part of Orbit
//
// Created on 2019-07-11 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_map_h
#define orbit_map_h

#include <value.h>

typedef struct sOrbitMap OrbitMap;

typedef struct {
    OrbitValue key;
    OrbitValue value;
} OrbitPair;

struct sOrbitMap {
    size_t count;
    size_t capacity;
    OrbitMapEntry* data;
};

void orbitMapInit(OrbitMap* self);
void orbitMapDeinit(OrbitGC* gc, OrbitMap* self);

void orbitMapInsert(OrbitGC* gc, OrbitMap* self, OrbitPair pair);
OrbitValue* orbitMapGet(OrbitGC* gc, OrbitMap* self, OrbitValue key);

#endif