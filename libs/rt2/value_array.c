//===--------------------------------------------------------------------------------------------===
// value_array.c - Dynamic array implementation
// This source is part of Orbit
//
// Created on 2019-06-27 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/rt2/memory.h>
#include <orbit/rt2/value_array.h>
#include <assert.h>

void orbit_arrayInit(OrbitArray* self) {
    assert(self && "null array error");
    self->capacity = 0;
    self->count = 0;
    self->data = NULL;
}

void orbit_arrayDeinit(OrbitArray* self) {
    assert(self && "null array error");
    DEALLOC_ARRAY(self->data, OrbitValue, self->capacity);
    orbit_arrayInit(self);
}

static void _arrayEnsure(OrbitArray* self, size_t required) {
    if(required < self->capacity)
        return;
    
    size_t newCapacity = self->capacity;
    while(newCapacity < required)
        newCapacity = GROW_CAPACITY(self->capacity);
    
    self->data = REALLOC_ARRAY(self->data, OrbitValue, self->capacity, newCapacity);
    self->capacity = newCapacity;
}

void orbit_arrayAppend(OrbitArray* self, OrbitValue item) {
    assert(self && "null array error");
    _arrayEnsure(self, self->count + 1);
    
    self->data[self->count] = item;
    self->count += 1;
}
