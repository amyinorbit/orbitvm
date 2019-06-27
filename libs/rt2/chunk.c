//===--------------------------------------------------------------------------------------------===
// chunk.c - Implementation of the bytecode chunk API
// This source is part of Orbit
//
// Created on 2019-06-27 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/rt2/chunk.h>
#include <orbit/rt2/memory.h>
#include <assert.h>

void orbit_chunkInit(OrbitChunk* self) {
    assert(self && "null chunk error");
    self->capacity = 0;
    self->count = 0;
    self->code = NULL;
    self->lines = NULL;
    orbit_arrayInit(&self->constants);
}

void orbit_chunkDeinit(OrbitChunk* self) {
    assert(self && "null chunk error");
    DEALLOC_ARRAY(self->code, uint8_t, self->capacity);
    DEALLOC_ARRAY(self->lines, OrbitLineData, self->capacity);
    orbit_arrayDeinit(&self->constants);
    orbit_chunkInit(self);
}

static void _chunkEnsure(OrbitChunk* self, size_t required) {
    size_t newCapacity = self->capacity;
    while(newCapacity < required)
        newCapacity = GROW_CAPACITY(self->capacity);
    
    self->code = REALLOC_ARRAY(self->code, uint8_t, self->capacity, newCapacity);
    self->lines = REALLOC_ARRAY(self->lines, OrbitLineData, self->capacity, newCapacity);
    self->capacity = newCapacity;
}

void orbit_chunkWrite(OrbitChunk* self, uint8_t byte, uint32_t line) {
    assert(self && "null chunk error");
    _chunkEnsure(self, self->count + 1);
    
    self->code[self->count] = byte;
    self->lines[self->count] = line;
    self->count += 1;
}

