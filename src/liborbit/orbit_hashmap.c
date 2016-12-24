//
//  orbit_hashmap.c
//  OrbitVM
//
//  Created by Cesar Parent on 22/11/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include "orbit_hashmap.h"
#include "orbit_utils.h"

#ifndef __LP64__
#define MAP_HASH(sel, mask) (((uintptr_t)(sel)>>2) & (mask))
#else
#define MAP_HASH(sel, mask) (((unsigned int)((uintptr_t)(sel)>>0)) & (mask))
#endif

static HashMap* orbit_hashmapGrow(HashMap* map) {
    OASSERT(map != NULL, "Null instance error");
    
    size_t newCapacity = map->capacity << 1;
    HashMap* bigmap = REALLOC(NULL, sizeof(HashMap)
                                    + (newCapacity * sizeof(HashBucket)));
    bigmap->capacity = newCapacity;
    bigmap->size = 0;
    bigmap->mask = newCapacity - 1;
    
    for(size_t i = 0; i < map->capacity; ++i) {
        if(!map->data[i].used) continue;
        size_t index = MAP_HASH(map->data[i].key, bigmap->mask);
        while(bigmap->data[index].used) {
            index = (index + 1) & bigmap->mask;
        }
        bigmap->data[index].used = true;
        bigmap->data[index].key = map->data[i].key;
        bigmap->data[index].data = map->data[i].data;
        bigmap->size += 1;
    }
    
    DEALLOC(map);
    return bigmap;
}

HashMap* orbit_hashmapNew(void) {
    HashMap* map = REALLOC(NULL, sizeof(HashMap)
                                 + (ORBIT_MAP_CAPACITY * sizeof(HashBucket)));
    map->mask = ORBIT_MAP_CAPACITY-1;
    map->capacity = ORBIT_MAP_CAPACITY;
    map->size = 0;
    return map;
}

HashMap* orbit_hashmapInsert(HashMap* map, const char* key, void* data) {
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
        map = orbit_hashmapGrow(map);
    }
    return map;
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

void orbit_hashmapDealloc(HashMap* map) {
    OASSERT(map != NULL, "Null instance error");
    REALLOC(map, 0);
}