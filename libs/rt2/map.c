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
#include <orbit/utils/hashing.h>
#include <assert.h>

static uint32_t hashValue(OrbitValue value) {
    if(ORBIT_IS_STRING(value)) return ORBIT_AST_STRING(value)->hash;
    if(ORBIT_IS_INT(value)) return ORBIT_AS_INT(value); // TODO: this is really not good
    if(ORBIT_IS_FLOAT(value)) return 0; // TODO: find double hashing algo
    return 0; // TODO: return a hashed pointer?
}

void orbitMapInit(OrbitMap* self) {
    self->capacity = 0;
    self->count = 0;
    self->data = NULL;
}

void orbitMapDeinit(OrbitGC* gc, OrbitMap* self) {
    
    orbitMapInit(self);
}

void orbitMapInsert(OrbitGC* gc, OrbitMap* self, OrbitPair pair) {
    uint32_t hash = hashValue(pair.left);
}

OrbitValue* orbitMapGet(OrbitGC* gc, OrbitMap* self, OrbitValue key) {
    
}

OrbitValue* orbitMapDelete(OrbitGC* gc, OrbitMap* self, OrbitValue key) {
    
}

