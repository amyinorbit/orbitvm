//===--------------------------------------------------------------------------------------------===
// orbit/utils/rcmap.c
// This source is part of Orbit - Utils
//
// Created on 2017-11-21 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <string.h>
#include <orbit/utils/assert.h>
#include <orbit/csupport/rcmap.h>

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
        //ORCRELEASE(map->data[i].key);
        ORCRELEASE(map->data[i].value);
    }
}

ORCMap* orbit_rcMapInit(ORCMap* map) {
    OASSERT(map != NULL, "Null instance error");
    //ORCINIT(map, &orbit_rcMapDeinit);
    
    map->data = ORBIT_ALLOC_ARRAY(ORCMapEntry, ORCMAP_DEFAULT_CAPACITY);
    map->size = 0;
    map->capacity = ORCMAP_DEFAULT_CAPACITY;
    
    ORCMAP_EACH(map) {
        map->data[i].key = orbit_invalidStringID;
        map->data[i].value = NULL;
    }
    return map;
}

// static inline bool _rcMapCompare(UTFConstString* a, UTFConstString* b) {
//     OASSERT(a != NULL, "Null reference error");
//     OASSERT(b != NULL, "Null reference error");
//     return a == b
//         || (a->hash == b->hash 
//             && a->length == b->length
//             && memcmp(a->data, b->data, a->length) == 0);
// }

static ORCMapEntry* _rcMapFindSlot(ORCMap* map, OCStringID keyID) {
    ORCMapEntry* insert = NULL;
    OCString* key = orbit_stringPoolGet(keyID);
    uint64_t index = key->hash % map->capacity;
    uint64_t start = index;
    
    do {
        if(map->data[index].key == orbit_invalidStringID) {
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
    
    OASSERT(insert != NULL, "Map wasn't grown when required");
    return insert;
}

// static ORCMapEntry* _rcMapFindSlotP(ORCMap* map, OCStringID id) {
//     OCString* key = orbit_stringPoolGet(id);
//     return _rcMapFindSlot(map, orbit_stringPoolGet(id));
// }

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
        map->data[i].key = orbit_invalidStringID;
        map->data[i].value = NULL;
    }
    
    for(uint64_t i = 0; i < oldCapacity; ++i) {
        if(oldData[i].key == orbit_invalidStringID) { continue; }
        orbit_rcMapInsertP(map, oldData[i].key, oldData[i].value);
        ORCRELEASE(oldData[i].value);
    }
}

void orbit_rcMapInsertP(ORCMap* map, OCStringID key, void* item) {
    OASSERT(map != NULL, "Null instance error");
    OASSERT(key != orbit_invalidStringID, "Null key error");
    
    if(map->size > ORCMAP_GROWTH_THRESHOLD * map->capacity) {
        _rcMapGrow(map);
    }
    
    ORCMapEntry* entry = _rcMapFindSlot(map, key);
    if(entry->key == orbit_invalidStringID) {
        map->size += 1;
    }
    entry->key = key;
    entry->value = ORCRETAIN(item);
}

void orbit_rcMapRemoveP(ORCMap* map, OCStringID key) {
    OASSERT(map != NULL, "Null instance error");
    OASSERT(key != orbit_invalidStringID, "Null key error");
    
    ORCMapEntry* slot = _rcMapFindSlot(map, key);
    if(slot->key == orbit_invalidStringID) { return; }
    
    //ORCRELEASE(slot->key);
    ORCRELEASE(slot->value);
    slot->key = orbit_invalidStringID;
    slot->value = (void*)tombstone;
    map->size -= 1;
}

void* orbit_rcMapGetP(ORCMap* map, OCStringID key) {
    OASSERT(map != NULL, "Null instance error");
    OASSERT(key != orbit_invalidStringID, "Null key error");
    
    ORCMapEntry* slot = _rcMapFindSlot(map, key);
    return slot->key != orbit_invalidStringID ? slot->value : NULL;
}
