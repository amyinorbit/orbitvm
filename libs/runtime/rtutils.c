//===--------------------------------------------------------------------------------------------===
// orbit/runtime/utils.c
// This source is part of Orbit - Runtime
//
// Created on 2016-12-06 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/utils/assert.h>
#include <orbit/runtime/rtutils.h>
#include <orbit/runtime/vm.h>
#include <orbit/runtime/gc.h>

void* orbit_allocator(OrbitVM* vm, void* ptr, size_t newSize) {
    OASSERT(vm != NULL, "Null instance error");
    vm->allocated += newSize;
    if(vm->allocated > vm->nextGC) {
        orbit_gcRun(vm);
    }
    
    if(newSize == 0) {
        free(ptr);
        return NULL;
    }
    void* mem = realloc(ptr, newSize);
    OASSERT(mem != NULL, "Error reallocating memory");
    return mem;
}

typedef union {
    double      number;
    uint32_t    raw[2];
} RawDouble;

// TODO: Move hashDouble to orbit/utils/hashing

uint32_t orbit_hashDouble(double number) {
    RawDouble bits = {.number = number};
    return bits.raw[0] ^ bits.raw[1];
}
