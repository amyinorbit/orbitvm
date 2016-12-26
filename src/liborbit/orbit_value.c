//
//  orbit_value.c
//  OrbitVM
//
//  Created by Cesar Parent on 26/12/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include "orbit_value.h"
#include "orbit_utils.h"

uint32_t orbit_hashString(const char* string, size_t length) {
    OASSERT(string != NULL, "Null instance error");
    
    //Fowler-Noll-Vo 1a hash
    //http://create.stephan-brumme.com/fnv-hash/
    uint32_t hash = 0x811C9DC5;
    for(size_t i = 0; i < length; ++i) {
        hash ^= string[i];
        hash *= 0x01000193;
    }
    return hash;
}

uint32_t orbit_hashObject(VMInstance* object) {
    return ((uint64_t)object >> 32) ^ ((uint64_t)object & 0x0000FFFF);
}


