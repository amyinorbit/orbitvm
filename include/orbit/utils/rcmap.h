//
//  orbit/utils/rcmap.h
//  Orbit - Utils
//
//  Created by Amy Parent on 2017-11-21.
//  Copyright © 2016-2017 Amy Parent. All rights reserved.
//
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
    ORCMapEntry     data[ORBIT_FLEXIBLE_ARRAY_MEMB];
};

ORCMap* orbit_rcMapNew();

void orbit_rcMapInsert(ORCMap** map, UTFConstString* key, void* item);
void orbit_rcMapRemove(ORCMap** map, UTFConstString* key);
void* orbit_rcMapGet(ORCMap* map, UTFConstString* key);


#endif /* orbit_utils_rcmap_h */
