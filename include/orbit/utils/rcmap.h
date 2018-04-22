//===--------------------------------------------------------------------------------------------===
// orbit/utils/rcmap.h
// This source is part of Orbit - Utils
//
// Created on 2017-11-21 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_utils_rcmap_h
#define orbit_utils_rcmap_h

#include <stdint.h>
#include <orbit/utils/platforms.h>
#include <orbit/utils/memory.h>
#include <orbit/utils/string.h>

typedef struct _ORCMap ORCMap;
typedef struct _ORCMapEntry ORCMapEntry;

struct _ORCMapEntry {
    UTFConstString* key;
    void*           value;
};

struct _ORCMap {
    uint64_t        size;
    uint64_t        capacity;
    ORCMapEntry*    data;
};

void orbit_rcMapDeinit(void* ref);
ORCMap* orbit_rcMapInit(ORCMap* map);

void orbit_rcMapInsert(ORCMap* map, UTFConstString* key, void* item);
void orbit_rcMapRemove(ORCMap* map, UTFConstString* key);
void* orbit_rcMapGet(ORCMap* map, UTFConstString* key);


#endif /* orbit_utils_rcmap_h */
