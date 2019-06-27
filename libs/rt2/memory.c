//===--------------------------------------------------------------------------------------------===
// orbit/runtime/utils.c
// This source is part of Orbit - Runtime
//
// Created on 2016-12-06 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <stdlib.h>
#include <orbit/rt2/memory.h>

void* orbit_allocator(void* ptr, size_t oldSize, size_t newSize) {
    if(newSize == 0) {
        free(ptr);
        return NULL;
    }
    
    void* mem = realloc(ptr, newSize);
    assert(mem != NULL && "Error reallocating memory");
    return mem;
}
