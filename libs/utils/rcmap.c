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

void orbit_rcMapDeinit(void* ref) {
    ORCMap* map = (ORCMap*)ref;
    for(uint64_t i = 0; i < map->capacity; ++i) {
        ORCRELEASE(map->data[i].key);
        ORCRELEASE(map->data[i].value);
    }
}

ORCMap* orbit_rcMapInit(ORCMap* map) {
    OASSERT(map != NULL, "Null instance error");
    ORCINIT(map, &orbit_rcMapDeinit);
    
    map->data = ORBIT_ALLOC_ARRAY(ORCMapEntry, ORCMAP_DEFAULT_CAPACITY);
    map->size = 0;
    map->capacity = ORCMAP_DEFAULT_CAPACITY;
    
    ORCMAP_EACH(map) {
        map->data[i].key = NULL;
        map->data[i].value = NULL;
    }
    return map;
}

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
            if(map->data[index].value == tombstone) {
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

static void _rcMapGrow(ORCMap* map) {
    OASSERT(map != NULL, "Null instance error");
    uint64_t oldCapacity = map->capacity;
    ORCMapEntry* oldData = map->data;
    
    if(map->capacity == 0) {
        map->capacity = ORCMAP_DEFAULT_CAPACITY;
    } else {
        map->capacity = map->capacity << 1;
    }
    
    map->data = ORBIT_ALLOC_ARRAY(ORCMapEntry, map->capacity);
    map->size = 0;
    
    for(uint64_t i = 0; i < map->capacity; ++i) {
        map->data[i].key = NULL;
        map->data[i].value = NULL;
    }
    
    for(uint64_t i = 0; i < oldCapacity; ++i) {
        if(oldData[i].key == NULL) { continue; }
        orbit_rcMapInsert(map, oldData[i].key, oldData[i].value);
        ORCRELEASE(oldData[i].value);
    }
}

void orbit_rcMapInsert(ORCMap* map, UTFConstString* key, void* item) {
    OASSERT(map != NULL, "Null instance error");
    OASSERT(key != NULL, "Null key error");
    
    if(map->size > ORCMAP_GROWTH_THRESHOLD * map->capacity) {
        // GROW MAP
        _rcMapGrow(map);
    }
    
    ORCMapEntry* entry = _rcMapFindSlot(map, key);
    if(entry->key == NULL) {
        map->size += 1;
    }
    entry->key = ORCRETAIN(key);
    entry->value = ORCRETAIN(item);
}

void orbit_rcMapRemove(ORCMap* map, UTFConstString* key) {
    OASSERT(map != NULL, "Null instance error");
    OASSERT(key != NULL, "Null key error");
    
    ORCMapEntry* slot = _rcMapFindSlot(map, key);
    if(slot->key == NULL) { return; }
    
    ORCRELEASE(slot->key);
    ORCRELEASE(slot->value);
    slot->key = NULL;
    slot->value = (void*)tombstone;
    map->size -= 1;
}

void* orbit_rcMapGet(ORCMap* map, UTFConstString* key) {
    OASSERT(map != NULL, "Null instance error");
    OASSERT(key != NULL, "Null key error");
    
    ORCMapEntry* slot = _rcMapFindSlot(map, key);
    return slot->key ? slot->value : NULL;
}
