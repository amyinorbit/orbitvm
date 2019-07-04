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
#include <orbit/rt2/garbage.h>

void* orbit_gcalloc(OrbitGC* gc, void* ptr, size_t oldSize, size_t newSize) {
    assert(gc && "can't GC-allocate with a null collector");
    gc->allocated += (newSize - oldSize);
    return orbit_allocator(ptr, oldSize, newSize);
}
