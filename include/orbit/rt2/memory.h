//===--------------------------------------------------------------------------------------------===
// orbit/runtime/utils.h
// This source is part of Orbit - Runtime
//
// Created on 2016-12-06 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_utils_h
#define orbit_utils_h

#include <orbit/common.h>

#define ALLOC(T) \
    orbit_allocator(NULL, 0, sizeof(T))

#define ALLOC_OBJECT(vm, T, kind) \
    (T*)orbit_objectNew(vm, kind, sizeof(T))
        
#define ALLOC_ARRAY(T, count) \
    orbit_allocator(NULL, 0, sizeof(T) * (count))

#define ALLOC_FLEX(T, U, count) \
    orbit_allocator(NULL, 0, sizeof(T) + (sizeof(U) * (count)))
        
#define REALLOC_ARRAY(array, T, old, count) \
    orbit_allocator(array, sizeof(T) * (old), sizeof(T) * (count))
        
#define DEALLOC(ptr, T) \
    orbit_allocator(ptr, sizeof(T), 0)

#define DEALLOC_FLEX(ptr, T, U, count) \
    orbit_allocator(ptr, sizeof(T) + (sizeof(U) * count), 0)
        
#define DEALLOC_ARRAY(ptr, T, count) \
    orbit_allocator(ptr, sizeof(T) * count, 0)

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

void* orbit_allocator(void* ptr, size_t oldSize, size_t newSize);

#endif /* orbit_utils_h */
