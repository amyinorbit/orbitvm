//===--------------------------------------------------------------------------------------------===
// orbit/utils/memory.c
// This source is part of Orbit - Utils
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdio.h>
#include <stdlib.h>
#include <orbit/utils/memory.h>

noreturn void orbit_die(const char* message) {
    fprintf(stderr, "fatal error: %s\n", message);
    abort();
}

void* orbit_allocator(void* ptr, size_t oldSize, size_t newSize) {
    if(newSize == 0) {
        free(ptr);
        return NULL;
    }
    
    void* mem = realloc(ptr, newSize);
    if(!mem) orbit_die("Error reallocating memory");
    return mem;
}
