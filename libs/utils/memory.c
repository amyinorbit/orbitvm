//===--------------------------------------------------------------------------------------------===
// orbit/utils/memory.c
// This source is part of Orbit - Utils
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdio.h>
#include <orbit/utils/memory.h>

noreturn void orbit_die(const char* message) {
    fprintf(stderr, "fatal error: %s\n", message);
    abort();
}

void* orbit_alloc(size_t size) {
    void* mem = malloc(size);
    if(!mem) { orbit_die("out of memory");  }
    return mem;
}

void* orbit_allocMulti(size_t count, size_t size) {
    void* mem = calloc(count, size);
    if(!mem) { orbit_die("out of memory");  }
    return mem;
}

void* orbit_realloc(void* memory, size_t size) {
    void* mem = realloc(memory, size);
    if(!mem) { orbit_die("out of memory");  }
    return mem;
}

void orbit_dealloc(void* memory) {
    if(!memory) { return; }
    free(memory);
}
