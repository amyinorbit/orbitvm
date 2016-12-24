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
    
    HashMap* bigmap = ALLOC_FLEX(sizeof(HashMap),
                                 sizeof(HashBucket),
                                 map->capacity << 2);
    bigmap->capacity = map->capacity << 2;
    bigmap->size = 0;
    bigmap->mask = bigmap->capacity - 1;
    
    for(size_t i = 0; i < map->capacity; ++i) {
        if(!map->data[i].used) continue;
        size_t index = MAP_HASH(map->data[i].key, bigmap->mask);
        while(bigmap->data[index].used) {
            index = (index + 1) & bigmap->mask;
        }
        bigmap->data[index].used = true;
        bigmap->data[index].key = map->data[i].key;
        bigmap->data[index].data = map->data[i].data;
    }
    
    DEALLOC(map);
    return bigmap;
}

HashMap* orbit_hashmapNew(void) {
    HashMap* map = ALLOC_FLEX(sizeof(HashMap),
                              sizeof(HashBucket),
                              ORBIT_MAP_DEFAULT_SIZE);
    map->mask = ORBIT_MAP_DEFAULT_SIZE-1;
    map->capacity = ORBIT_MAP_DEFAULT_SIZE;
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
    DEALLOC(map);
}