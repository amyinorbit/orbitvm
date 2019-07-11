//===--------------------------------------------------------------------------------------------===
// value_map.h - Orbit Runtime 2.0's hash table type
// This source is part of Orbit
//
// Created on 2019-07-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_value_map_h
#define orbit_value_map_h

#include <orbit/common.h>
#include <orbit/rt2/value.h>

typedef struct sOrbitMap OrbitMap;
typedef OrbitString* OrbitMapKey;

typedef OrbitMapEntry {
    OrbitMapKey key;
    OrbitValue value;
};

struct sOrbitMap {
    size_t capacity;
    size_t count;
    OrbitMapEntry* data;
};

void orbitMapInit(OrbitMap* self);
void orbitMapDeinit(OrbitMap* self);

OrbitValue orbitMapGet(const OrbitMap* self, OrbitMapKey key);
void orbitMapInsert(OrbitMap* self, OrbitMapKey key, OrbitValue value);
void orbitMapRemove(OrbitMap* self, OrbitMapKey key);

#endif
