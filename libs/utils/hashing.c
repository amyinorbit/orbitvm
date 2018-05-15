//===--------------------------------------------------------------------------------------------===
// hashing.c - Implementations of hashing algorithms for data types used by orbit
// This source is part of Orbit
//
// Created on 2018-05-08 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <stddef.h> // in order to get NULL
#include <orbit/utils/hashing.h>

uint32_t orbit_hashString(const char* string, uint64_t length) {
    assert(string != NULL && "Null instance error");
    
    //Fowler-Noll-Vo 1a hash
    //http://create.stephan-brumme.com/fnv-hash/
    uint32_t hash = 0x811C9DC5;
    for(uint64_t i = 0; i < length; ++i) {
        hash = (hash ^ string[i]) * 0x01000193;
    }
    return hash;
}

typedef union {
    double      number;
    uint32_t    raw[2];
} RawDouble;

uint32_t orbit_hashDouble(double number) {
    RawDouble bits = {.number = number};
    return bits.raw[0] ^ bits.raw[1];
}
