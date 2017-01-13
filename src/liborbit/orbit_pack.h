//
//  orbit_packfile.h
//  OrbitVM
//
//  Created by Cesar Parent on 2017-01-12.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#ifndef orbit_pack_h
#define orbit_pack_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool orbit_pack8(FILE* out, uint8_t bits);

bool orbit_pack16(FILE* out, uint16_t bits);

bool orbit_pack32(FILE* out, uint32_t bits);

bool orbit_pack64(FILE* out, uint64_t bits);

bool orbit_packIEEE754(FILE* out, double bits);

bool orbit_packBytes(FILE* out, uint8_t* bytes, size_t count);


bool orbit_unpack8(FILE* in, uint8_t* out);

bool orbit_unpack16(FILE* in, uint16_t* out);

bool orbit_unpack32(FILE* in, uint32_t* out);

bool orbit_unpack64(FILE* in, uint64_t* out);

bool orbit_unpackIEEE754(FILE* in, double* out);

bool orbit_unpackBytes(FILE* in, uint8_t* bytes, size_t count);

#endif /* orbit_pack_h */
