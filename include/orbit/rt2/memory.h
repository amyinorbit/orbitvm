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

#define ALLOC(type) \
    orbit_allocator(NULL, 0, sizeof(type))
        
#define ALLOC_ARRAY(type, count) \
    orbit_allocator(NULL, 0, sizeof(type) * (count))
        
#define REALLOC_ARRAY(array, type, old, count) \
    orbit_allocator(array, sizeof(type) * (old), sizeof(type) * (count))
        
#define DEALLOC(ptr, type) \
    orbit_allocator(ptr, sizeof(type), 0)
        
#define DEALLOC_ARRAY(ptr, type, count) \
    orbit_allocator(ptr, sizeof(type) * count, 0)

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

void* orbit_allocator(void* ptr, size_t oldSize, size_t newSize);

#endif /* orbit_utils_h */
