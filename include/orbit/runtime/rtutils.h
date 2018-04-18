//===--------------------------------------------------------------------------------------------===
// orbit/runtime/utils.h
// This source is part of Orbit - Runtime
//
// Created on 2016-12-06 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_utils_h
#define orbit_utils_h

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <orbit/orbit.h>

#define ALLOC(vm, type) \
    orbit_allocator(vm, NULL, sizeof(type))
#define ALLOC_ARRAY(vm, type, count) \
    orbit_allocator(vm, NULL, sizeof(type) * (count))
#define ALLOC_FLEX(vm, type, arrayType, count) \
    orbit_allocator(vm, NULL, sizeof(type) + (sizeof(arrayType) * (count)))
#define REALLOC_ARRAY(vm, array, type, count) \
    orbit_allocator(vm, array, (sizeof(type) * (count)))
#define DEALLOC(vm, ptr) \
    orbit_allocator(vm, ptr, 0)

// Single function used for memory allocation and deallocation in orbit.
void* orbit_allocator(OrbitVM* vm, void* ptr, size_t newSize);

// Hash functions

// Computes the hash code of [number].
uint32_t orbit_hashDouble(double number);

#endif /* orbit_utils_h */
