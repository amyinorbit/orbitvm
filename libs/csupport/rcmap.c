//===--------------------------------------------------------------------------------------------===
// orbit/utils/rcmap.c
// This source is part of Orbit - Utils
//
// Created on 2017-11-21 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <string.h>
#include <orbit/csupport/rcmap.h>

#define ORCMAP_DEFAULT_CAPACITY 32
#define ORCMAP_GROWTH_THRESHOLD 0.75
#define ORCMAP_GROWTH_FACTOR
#define ORCMAP_EACH(map) for(uint64_t i = 0; i < map->capacity; ++i)

// We use this to mark deletions, so that we can delete in O(1), and we'll remove tombstones
// when we grow/shrink the map.
static const char* tombstone = "ORCMAP_TOMBSTONE";

void orbitRcMapDeinit(void* ref) {
    ORCMap* map = (ORCMap*)ref;
    for(uint64_t i = 0; i < map->capacity; ++i) {
        //ORCRELEASE(map->data[i].key);
        ORCRELEASE(map->data[i].value);
    }
    ORBIT_DEALLOC_ARRAY(map->data, ORCMapEntry, map->capacity);
}

ORCMap* orbitRcMapInit(ORCMap* map) {
    assert(map != NULL && "Null instance error");
    //ORCINIT(map, &orbitRcMapDeinit);
    
    map->data = ORBIT_ALLOC_ARRAY(ORCMapEntry, ORCMAP_DEFAULT_CAPACITY);
    map->size = 0;
    map->capacity = ORCMAP_DEFAULT_CAPACITY;
    
    ORCMAP_EACH(map) {
        map->data[i].key = orbitInvalidStringID;
        map->data[i].value = NULL;
    }
    return map;
}

// static inline bool _rcMapCompare(UTFConstString* a, UTFConstString* b) {
//     assert(a != NULL && "Null reference error");
//     assert(b != NULL && "Null reference error");
//     return a == b
//         || (a->hash == b->hash 
//             && a->length == b->length
//             && memcmp(a->data, b->data, a->length) == 0);
// }

static ORCMapEntry* _rcMapFindSlot(ORCMap* map, OCStringID keyID) {
    ORCMapEntry* insert = NULL;
    OCString* key = orbitStringPoolGet(keyID);
    uint64_t index = key->hash % map->capacity;
    uint64_t start = index;
    
    do {
        if(map->data[index].key == orbitInvalidStringID) {
            if(map->data[index].value == tombstone) {
                if(insert == NULL) { insert = &map->data[index]; }
            } else {
                return &map->data[index];
            }
        } else {
            if(map->data[index].key == keyID) {
                return &map->data[index];
            }
        }
        index = (index + 1) % map->capacity;
    } while(index != start);
    
    assert(insert != NULL && "Map wasn't grown when required");
    return insert;
}

// static ORCMapEntry* _rcMapFindSlotP(ORCMap* map, OCStringID id) {
//     OCString* key = orbitStringPoolGet(id);
//     return _rcMapFindSlot(map, orbitStringPoolGet(id));
// }

static void _rcMapGrow(ORCMap* map) {
    assert(map != NULL && "Null instance error");
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
        map->data[i].key = orbitInvalidStringID;
        map->data[i].value = NULL;
    }
    
    for(uint64_t i = 0; i < oldCapacity; ++i) {
        if(oldData[i].key == orbitInvalidStringID) { continue; }
        orbitRcMapInsertP(map, oldData[i].key, oldData[i].value);
        ORCRELEASE(oldData[i].value);
    }
    ORBIT_DEALLOC_ARRAY(oldData, ORCMapEntry, oldCapacity);
}

void orbitRcMapInsertP(ORCMap* map, OCStringID key, void* item) {
    assert(map != NULL && "Null instance error");
    assert(key != orbitInvalidStringID && "Null key error");
    
    if(map->size > ORCMAP_GROWTH_THRESHOLD * map->capacity) {
        _rcMapGrow(map);
    }
    
    ORCMapEntry* entry = _rcMapFindSlot(map, key);
    if(entry->key == orbitInvalidStringID) {
        map->size += 1;
    }
    entry->key = key;
    entry->value = ORCRETAIN(item);
}

void orbitRcMapRemoveP(ORCMap* map, OCStringID key) {
    assert(map != NULL && "Null instance error");
    assert(key != orbitInvalidStringID && "Null key error");
    
    ORCMapEntry* slot = _rcMapFindSlot(map, key);
    if(slot->key == orbitInvalidStringID) { return; }
    
    //ORCRELEASE(slot->key);
    ORCRELEASE(slot->value);
    slot->key = orbitInvalidStringID;
    slot->value = (void*)tombstone;
    map->size -= 1;
}

void* orbitRcMapGetP(ORCMap* map, OCStringID key) {
    assert(map != NULL && "Null instance error");
    assert(key != orbitInvalidStringID && "Null key error");
    
    ORCMapEntry* slot = _rcMapFindSlot(map, key);
    return slot->key != orbitInvalidStringID ? slot->value : NULL;
}
