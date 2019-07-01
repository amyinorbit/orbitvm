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

void orbit_mapInit(OrbitMap* self);
void orbit_mapDeinit(OrbitMap* self);

OrbitValue orbit_mapGet(const OrbitMap* self, OrbitMapKey key);
void orbit_mapInsert(OrbitMap* self, OrbitMapKey key, OrbitValue value);
void orbit_mapRemove(OrbitMap* self, OrbitMapKey key);

#endif
