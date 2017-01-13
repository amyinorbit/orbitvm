//
//  orbit_objfile.h
//  OrbitVM
//
//  Created by Cesar Parent on 2017-01-12.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#ifndef orbit_objfile_h
#define orbit_objfile_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool orbit_objWrite8(FILE* out, uint8_t bits);

bool orbit_objWrite16(FILE* out, uint16_t bits);

bool orbit_objWrite32(FILE* out, uint32_t bits);

bool orbit_objWrite64(FILE* out, uint64_t bits);

bool orbit_objWriteIEEE754(FILE* out, double bits);

bool orbit_objWriteBytes(FILE* out, uint8_t* bytes, size_t count);


bool orbit_objRead8(FILE* in, uint8_t* out);

bool orbit_objRead16(FILE* in, uint16_t* out);

bool orbit_objRead32(FILE* in, uint32_t* out);

bool orbit_objRead64(FILE* in, uint64_t* out);

bool orbit_objReadIEEE754(FILE* in, double* out);

bool orbit_objReadBytes(FILE* in, uint8_t* bytes, size_t count);

#endif /* orbit_objfile_h */
