//
//  orbit_utils.c
//  OrbitVM
//
//  Created by Cesar Parent on 06/12/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include "orbit_utils.h"

void* orbit_alloc(size_t size) {
    void* mem = calloc(size, 1);
    OASSERT(mem != NULL, "Error allocating memory (%zu bytes)", size);
    return mem;
}

void* orbit_allocFlex(size_t size, size_t arraySize, size_t count) {
    void* mem = calloc(size + arraySize * count, 1);
    OASSERT(mem != NULL, "Error allocating memory (%zu bytes)", size + arraySize * count);
    return mem;
}

void* orbit_realloc(void* ptr, size_t newSize) {
    void* mem = realloc(ptr, newSize);
    OASSERT(mem != NULL, "Error reallocating memory (%zu bytes)", newSize);
    return mem;
}

void orbit_dealloc(void* ptr) {
    free(ptr);
}
