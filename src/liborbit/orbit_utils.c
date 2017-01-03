//
//  orbit_utils.c
//  OrbitVM
//
//  Created by Cesar Parent on 2016-12-06.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include "orbit_utils.h"
#include "orbit_vm.h"

void* orbit_allocator(OrbitVM* vm, void* ptr, size_t newSize) {
    if(newSize == 0) {
        free(ptr);
        return NULL;
    }
    void* mem = realloc(ptr, newSize);
    OASSERT(mem != NULL, "Error reallocating memory");
    if(vm != NULL) vm->allocated += newSize;
    return mem;
}

typedef union {
    double      number;
    uint32_t    raw[2];
} RawDouble;

uint32_t orbit_hashString(const char* string, size_t length) {
    OASSERT(string != NULL, "Null instance error");
    
    //Fowler-Noll-Vo 1a hash
    //http://create.stephan-brumme.com/fnv-hash/
    uint32_t hash = 0x811C9DC5;
    for(size_t i = 0; i < length; ++i) {
        hash = (hash ^ string[i]) * 0x01000193;
    }
    return hash;
}

uint32_t orbit_hashDouble(double number) {
    RawDouble bits = {.number = number};
    return bits.raw[0] ^ bits.raw[1];
}
