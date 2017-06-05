//
//  orbit/utils/packfile.h
//  Orbit - Utils
//
//  Created by Amy Parent on 2017-01-12.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
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

OrbitPackError orbit_pack8(FILE* out, uint8_t bits);

OrbitPackError orbit_pack16(FILE* out, uint16_t bits);

OrbitPackError orbit_pack32(FILE* out, uint32_t bits);

OrbitPackError orbit_pack64(FILE* out, uint64_t bits);

OrbitPackError orbit_packIEEE754(FILE* out, double bits);

OrbitPackError orbit_packBytes(FILE* out, uint8_t* bytes, size_t count);


uint8_t orbit_unpack8(FILE* in, OrbitPackError* error);

uint16_t orbit_unpack16(FILE* in, OrbitPackError* error);

uint32_t orbit_unpack32(FILE* in, OrbitPackError* error);

uint64_t orbit_unpack64(FILE* in, OrbitPackError* error);

double orbit_unpackIEEE754(FILE* in, OrbitPackError* error);

OrbitPackError orbit_unpackBytes(FILE* in, uint8_t* bytes, size_t count);

#endif /* orbit_pack_h */
