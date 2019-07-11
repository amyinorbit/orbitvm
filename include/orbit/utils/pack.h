//===--------------------------------------------------------------------------------------------===
// orbit/utils/packfile.h
// This source is part of Orbit - Utils
//
// Created on 2017-01-12 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_pack_h
#define orbit_pack_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef enum {
    PACK_NOERROR    =  0,
    ERROR_PACK      = -1,
    ERROR_UNPACK    = -2,
} OrbitPackError;

OrbitPackError orbitPack8(FILE* out, uint8_t bits);

OrbitPackError orbitPack16(FILE* out, uint16_t bits);

OrbitPackError orbitPack32(FILE* out, uint32_t bits);

OrbitPackError orbitPack64(FILE* out, uint64_t bits);

OrbitPackError orbitPackIEEE754(FILE* out, double bits);

OrbitPackError orbitPackBytes(FILE* out, uint8_t* bytes, size_t count);


uint8_t orbitUnpack8(FILE* in, OrbitPackError* error);

uint16_t orbitUnpack16(FILE* in, OrbitPackError* error);

uint32_t orbitUnpack32(FILE* in, OrbitPackError* error);

uint64_t orbitUnpack64(FILE* in, OrbitPackError* error);

double orbitUnpackIEEE754(FILE* in, OrbitPackError* error);

OrbitPackError orbitUnpackBytes(FILE* in, uint8_t* bytes, size_t count);

#endif /* orbit_pack_h */
