//===--------------------------------------------------------------------------------------------===
// map.c - Hashmap implementation
// This source is part of Orbit
//
// Created on 2019-07-11 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/rt2/map.h>
#include <orbit/rt2/value_object.h>
#include <orbit/rt2/memory.h>
#include <orbit/utils/hashing.h>
#include <assert.h>

static uint32_t hashValue(OrbitValue value) {
    if(ORBIT_IS_STRING(value)) return ORBIT_AS_STRING(value)->hash;
    if(ORBIT_IS_INT(value)) return ORBIT_AS_INT(value); // TODO: this is really not good
    if(ORBIT_IS_FLOAT(value)) return 0; // TODO: find double hashing algo
    return 0; // TODO: return a hashed pointer?
}

static OrbitPair* mapCreateData(OrbitGC* gc, size_t capacity) {
    OrbitPair* data = orbitGCalloc(gc, NULL, 0, sizeof(OrbitPair) * capacity);
    for(int i = 0; i < capacity; ++i) {
        data[i].left = ORBIT_VALUE_NIL;
        data[i].right = ORBIT_VALUE_NIL;
    }
    return data;
}

static bool mapCompare(OrbitValue lhs, OrbitValue rhs) {
    return orbitValueEquals(lhs, rhs); //
}

static OrbitPair* mapFindSlot(OrbitMap* self, OrbitValue key) {
    OrbitPair* insert = NULL;
    uint32_t i = hashValue(key) % (self->capacity);
    
    uint32_t start = i;
    
    do {
        if(ORBIT_IS_NIL(self->data[i].left)) {
            if(ORBIT_IS_FALSE(self->data[i].right)) return &self->data[i];
            insert = insert ? insert : &self->data[i];
        } else {
            if(mapCompare(self->data[i].left, key)) return &self->data[i];
        }
        i = (i + 1) % (self->capacity);
    } while(i != start);
    assert(insert && "Map should not be full");
    return insert;
}

static void mapInsertFast(OrbitMap* self, OrbitPair pair) {
    OrbitPair* slot = mapFindSlot(self, pair.left);
    *slot = pair;
}

static void mapEnsure(OrbitGC* gc, OrbitMap* self, size_t required) {
    if(required < 0.7 * self->capacity) return;
    
    uint32_t oldCapacity = self->capacity;
    OrbitPair* oldData = self->data;
    while(0.7 * self->capacity < required) {
        self->capacity = ORBIT_GROW_CAPACITY(self->capacity);
    }
    
    self->data = mapCreateData(gc, self->capacity);
    for(int i = 0; i < oldCapacity; ++i) {
        if(ORBIT_IS_NIL(oldData[i].left)) continue;
        mapInsertFast(self, oldData[i]);
    }
    orbitGCalloc(gc, oldData, oldCapacity * sizeof(OrbitPair), 0);
}

void orbitMapInit(OrbitMap* self) {
    self->capacity = 0;
    self->count = 0;
    self->data = NULL;
}


void orbitMapDeinit(OrbitGC* gc, OrbitMap* self) {
    orbitGCalloc(gc, self->data, sizeof(OrbitPair) * self->capacity, 0);
    orbitMapInit(self);
}

void orbitMapInsert(OrbitGC* gc, OrbitMap* self, OrbitPair pair) {
    mapEnsure(gc, self, self->count+1);
    OrbitPair* slot = mapFindSlot(self, pair.left);
    if(ORBIT_IS_NIL(slot->left)) self->count += 1;
    *slot = pair;
}

bool orbitMapGet(OrbitGC* gc, OrbitMap* self, OrbitValue key, OrbitValue* result) {
    OrbitPair* slot = mapFindSlot(self, key);
    if(ORBIT_IS_NIL(slot->left)) {
        *result = ORBIT_VALUE_NIL;
        return false;
    }
    *result = slot->right;
    return true;
}

void orbitMapRemove(OrbitGC* gc, OrbitMap* self, OrbitValue key) {
    OrbitPair* slot = mapFindSlot(self, key);
    if(ORBIT_IS_NIL(slot->left)) return;
    slot->left = ORBIT_VALUE_NIL;
    slot->right = ORBIT_VALUE_TRUE;
    self->count -= 1;
}

