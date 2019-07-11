//===--------------------------------------------------------------------------------------------===
// orbit/utils/rcmap.h
// This source is part of Orbit - Utils
//
// Created on 2017-11-21 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_utils_rcmap_h
#define orbit_utils_rcmap_h

#include <stdint.h>
#include <orbit/csupport/string.h>
#include <orbit/utils/platforms.h>
#include <orbit/utils/memory.h>

typedef struct _ORCMap ORCMap;
typedef struct _ORCMapEntry ORCMapEntry;

struct _ORCMapEntry {
    //UTFConstString* key;
    OCStringID      key;
    void*           value;
};

struct _ORCMap {
    uint64_t        size;
    uint64_t        capacity;
    ORCMapEntry*    data;
};

void orbitRcMapDeinit(void* ref);
ORCMap* orbitRcMapInit(ORCMap* map);

//void orbitRcMapInsert(ORCMap* map, const char* key, uint64_t length, void* item);
void orbitRcMapInsertP(ORCMap* map, OCStringID key, void* item);
//void orbitRcMapRemove(ORCMap* map, const char* key, uint64_t length);
void orbitRcMapRemoveP(ORCMap* map, OCStringID key);
//void* orbitRcMapGet(ORCMap* map, const char* key, uint64_t length);
void* orbitRcMapGetP(ORCMap* map, OCStringID key);


#endif /* orbit_utils_rcmap_h */
