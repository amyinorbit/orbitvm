//
//  orbit_objfile.h
//  OrbitVM
//
//  Created by Cesar Parent on 2017-01-12.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#include "orbit_objfile.h"

bool orbit_objWrite8(FILE* out, uint8_t bits) {
    return fwrite(&bits, 1, 1, out) == 1;
}

bool orbit_objWrite16(FILE* out, uint16_t bits) {
    return orbit_objWrite8(out, bits >> 8) && orbit_objWrite8(out, bits & 0x00ff);
}

bool orbit_objWrite32(FILE* out, uint32_t bits) {
    uint8_t byte;
    for(int8_t i = 3; i >= 0; --i) {
        byte = bits >> (8*i);
        if(fwrite(&byte, 1, 1, out) != 1) return false;
    }
    return true;
}

bool orbit_objWrite64(FILE* out, uint64_t bits) {
    uint8_t byte;
    for(int8_t i = 7; i >= 0; --i) {
        byte = bits >> (8*i);
        if(fwrite(&byte, 1, 1, out) != 1) return false;
    }
    return true;
}

bool orbit_objWriteIEEE754(FILE* out, double bits) {
    // TODO: implementation
    return false;
}

bool orbit_objWriteBytes(FILE* out, uint8_t* bytes, size_t count) {
    return fwrite(bytes, 1, count, out) == count;
}


bool orbit_objRead8(FILE* in, uint8_t* out) {
    return fread(out, 1, 1, in) == 1;
}

bool orbit_objRead16(FILE* in, uint16_t* out) {
    uint8_t high = 0, low = 0;
    if(fread(&high, 1, 1, in) != 1) return false;
    if(fread(&low, 1, 1, in) != 1) return false;
    
    *out = ((uint16_t)high << 8) | (uint16_t)low;
    return true;
}

bool orbit_objRead32(FILE* in, uint32_t* out) {
    uint8_t byte = 0;
    *out = 0;
    
    for(int8_t i = 3; i >= 0; --i) {
        if(fread(&byte, 1, 1, in) != 1) return false;
        *out = (*out) | (uint32_t)(byte << (8*i));
    }
    return true;
}

bool orbit_objRead64(FILE* in, uint64_t* out) {
    uint8_t byte = 0;
    *out = 0;
    
    for(int8_t i = 7; i >= 0; --i) {
        if(fread(&byte, 1, 1, in) != 1) return false;
        *out = (*out) | (uint64_t)(byte << (8*i));
    }
    return true;
}

bool orbit_objReadIEE754(FILE* in, double* out) {
    // TODO: implementation
    return false;
}

bool orbit_objReadBytes(FILE* in, uint8_t* bytes, size_t count) {
    return fread(bytes, 1, count, in) == count;
}
