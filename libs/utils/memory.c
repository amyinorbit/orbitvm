//
//  orbit/utils/memory.c
//  Orbit - Utils
//
//  Created by Amy Parent on 2017-10-23.
//  Copyright © 2016-2017 Amy Parent. All rights reserved.
//
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
