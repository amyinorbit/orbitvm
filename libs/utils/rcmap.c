//
//  orbit/utils/rcmap.c
//  Orbit - Utils
//
//  Created by Amy Parent on 2017-11-21.
//  Copyright © 2016-2017 Amy Parent. All rights reserved.
//
#include <string.h>
#include <orbit/utils/assert.h>
#include <orbit/utils/rcmap.h>

#define ORCMAP_DEFAULT_CAPACITY 32
#define ORCMAP_GROWTH_THRESHOLD 0.75
#define ORCMAP_GROWTH_FACTOR
#define ORCMAP_EACH(map) for(uint64_t i = 0; i < map->capacity; ++i)

// We use this to mark deletions, so that we can delete in O(1), and we'll remove tombstones
// when we grow/shrink the map.
static const char* tombstone = "ORCMAP_TOMBSTONE";

void _rcMapDestructor(void* ref) {
    ORCMap* map = (ORCMap*)ref;
    for(uint64_t i = 0; i < map->capacity; ++i) {
        ORCRELEASE(map->data[i].key);
        ORCRELEASE(map->data[i].value);
    }
}

ORCMap* orbit_rcMapNew() {
    ORCMap* map = orbit_alloc(sizeof(ORCMap) + ORCMAP_DEFAULT_CAPACITY * sizeof(ORCMapEntry));
    ORCINIT(map, &_rcMapDestructor);
    map->size = 0;
    map->capacity = ORCMAP_DEFAULT_CAPACITY;
    
    ORCMAP_EACH(map) {
        map->data[i].key = NULL;
        map->data[i].value = NULL;
    }
    return map;
}
/*
    GCMapEntry* insert = NULL;
    uint32_t index = orbit_valueHash(key) & map->mask;
    uint32_t start = index;
    
    do {
        if(IS_NIL(map->data[index].key)) {
            if(IS_FALSE(map->data[index].value)) {
                // Empty slot, we're done, the key is not in the map.
                return &map->data[index];
            } else {
                // Tombstone, so we need to keep searching. Keep a pointer
                // to the tombstone so that this can be returned for insertion.
                if(insert == NULL) insert = &map->data[index];
            }
        } else {
            if(orbit_gcMapComp(key, map->data[index].key)) {
                return &map->data[index];
            }
        }
        index = (index + 1) & map->mask;
    } while(index != start);
    
    return insert;
*/

static inline bool _rcMapCompare(UTFConstString* a, UTFConstString* b) {
    OASSERT(a != NULL, "Null reference error");
    OASSERT(b != NULL, "Null reference error");
    return a == b
        || (a->hash == b->hash 
            && a->length == b->length
            && memcmp(a->data, b->data, a->length) == 0);
}

static ORCMapEntry* _rcMapFindSlot(ORCMap* map, UTFConstString* key) {
    ORCMapEntry* insert = NULL;
    uint64_t index = key->hash % map->capacity;
    uint64_t start = index;
    
    do {
        if(map->data[index].key == NULL) {
            if(map->data[index].value == &tombstone) {
                if(insert == NULL) { insert = &map->data[index]; }
            } else {
                return &map->data[index];
            }
        } else {
            if(_rcMapCompare(map->data[index].key, key)) {
                return &map->data[index];
            }
        }
        index = (index + 1) % map->capacity;
    } while(index != start);
    
    OASSERT(insert != NULL, "Map wasn't grown when required");
    return insert;
}

//static ORCMap* _rcMapReserve(ORCMap* map)

void orbit_rcMapInsert(ORCMap** ref, UTFConstString* key, void* item) {
    OASSERT(ref != NULL, "Null instance error");
    ORCMap* map = *ref;
    OASSERT(map != NULL, "Null instance error");
    
    if(map->size > ORCMAP_GROWTH_THRESHOLD * map->capacity) {
        // GROW MAP
    }
    
    ORCMapEntry* entry = _rcMapFindSlot(map, key);
    if(entry->key == NULL) {
        map->size += 1;
    }
    entry->key = ORCRETAIN(key);
    entry->value = ORCRETAIN(item);
    
    *ref = map;
}

void orbit_rcMapRemove(ORCMap** ref, UTFConstString* key) {
    OASSERT(ref != NULL, "Null instance error");
    ORCMap* map = *ref;
    OASSERT(map != NULL, "Null instance error");
    
}

void* orbit_rcMapGet(ORCMap* map, UTFConstString* key) {
    return NULL;
}
