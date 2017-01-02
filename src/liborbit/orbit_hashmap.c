//
//  orbit_hashmap.c
//  OrbitVM
//
//  Created by Cesar Parent on 22/11/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include <string.h>
#include "orbit_hashmap.h"
#include "orbit_utils.h"

#ifndef __LP64__
#define MAP_HASH(sel, mask) (((uintptr_t)(sel)>>2) & (mask))
#else
#define MAP_HASH(sel, mask) (((unsigned int)((uintptr_t)(sel)>>0)) & (mask))
#endif

static void orbit_hashmapGrow(HashMap* map) {
    OASSERT(map != NULL, "Null instance error");
    
    HashBucket* oldData = map->data;
    size_t oldCapacity = map->capacity;
    
    map->capacity = map->capacity << 1;
    map->mask = map->capacity - 1;
    map->data = ALLOC_ARRAY(HashBucket, map->capacity);
    memset(map->data, 0, map->capacity);
    
    for(size_t i = 0; i < oldCapacity; ++i) {
        if(!oldData[i].used) continue;
        size_t index = MAP_HASH(oldData[i].key, map->mask);
        while(map->data[index].used) {
            index = (index + 1) & map->mask;
        }
        map->data[index].used = true;
        map->data[index].key = oldData[i].key;
        map->data[index].data = oldData[i].data;
    }
    DEALLOC(oldData);
}

void orbit_hashmapInit(HashMap* map) {
    OASSERT(map != NULL, "Null instance error");
    
    map->data = ALLOC_ARRAY(HashBucket, ORBIT_MAP_CAPACITY);
    memset(map->data, 0, ORBIT_MAP_CAPACITY);
    
    map->mask = ORBIT_MAP_CAPACITY-1;
    map->capacity = ORBIT_MAP_CAPACITY;
    map->size = 0;
}

void orbit_hashmapInsert(HashMap* map, const char* key, void* data) {
    OASSERT(map != NULL, "Null instance error");
    size_t index = MAP_HASH(key, map->mask);
    while(map->data[index].used) {
        index = (index + 1) & map->mask;
    }
    map->data[index].key = key;
    map->data[index].used = true;
    map->data[index].data = data;
    map->size += 1;
    
    if(map->size >= 0.75 * map->capacity) {
        orbit_hashmapGrow(map);
    }
}

void* orbit_hashmapGet(HashMap* map, const char* key) {
    OASSERT(map != NULL, "Null instance error");
    for(size_t index = MAP_HASH(key, map->mask);
        map->data[index].used;
        index = (index +  1) & map->mask) {
        if(key == map->data[index].key) {
            return map->data[index].data;
        }
    }
    return NULL;
}

void orbit_hashmapDeinit(HashMap* map) {
    OASSERT(map != NULL, "Null instance error");
    DEALLOC(map->data);
}