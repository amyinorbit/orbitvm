//
//  orbit_pack.c
//  OrbitVM
//
//  Created by Cesar Parent on 2017-01-12.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#include "orbit_pack.h"

#define IEEE754_BITS 64
#define IEEE754_EXPBITS 11
#define IEEE754_SIGNIFICANDBITS (IEEE754_BITS - IEEE754_EXPBITS - 1)

// Barely adapted form `Beej's guide to network programming`
// http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#serialization
static uint64_t pack754(long double f)
{
    long double fnorm;
    int32_t shift;
    int64_t sign, exp, significand;

    if (f == 0.0) return 0; // get this special case out of the way

    // check sign and begin normalization
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }

    // get the normalized form of f and track the exponent
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;

    // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL<<IEEE754_SIGNIFICANDBITS) + 0.5f);

    // get the biased exponent
    exp = shift + ((1<<(IEEE754_EXPBITS-1)) - 1); // shift + bias

    // return the final answer
    return (sign<<(IEEE754_BITS-1)) | (exp<<(IEEE754_BITS-IEEE754_EXPBITS-1)) | significand;
}

// Barely adapted form `Beej's guide to network programming`
// http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#serialization
static long double unpack754(uint64_t i)
{
    long double result;
    int64_t shift;
    uint32_t bias;
    if (i == 0) return 0.0;

    // pull the significand
    result = (i&((1LL<<IEEE754_SIGNIFICANDBITS)-1)); // mask
    result /= (1LL<<IEEE754_SIGNIFICANDBITS); // convert back to float
    result += 1.0f; // add the one back on

    // deal with the exponent
    bias = (1<<(IEEE754_EXPBITS-1)) - 1;
    shift = ((i>>IEEE754_SIGNIFICANDBITS)&((1LL<<IEEE754_EXPBITS)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // sign it
    result *= (i>>(IEEE754_BITS-1))&1? -1.0: 1.0;
    return result;
}

bool orbit_pack8(FILE* out, uint8_t bits) {
    return fwrite(&bits, 1, 1, out) == 1;
}

bool orbit_pack16(FILE* out, uint16_t bits) {
    return orbit_pack8(out, bits >> 8) && orbit_pack8(out, bits & 0x00ff);
}

bool orbit_pack32(FILE* out, uint32_t bits) {
    uint8_t byte;
    for(int8_t i = 3; i >= 0; --i) {
        byte = bits >> (8*i);
        if(fwrite(&byte, 1, 1, out) != 1) return false;
    }
    return true;
}

bool orbit_pack64(FILE* out, uint64_t bits) {
    uint8_t byte;
    for(int8_t i = 7; i >= 0; --i) {
        byte = (bits >> (8*i)) & 0x00000000000000ff;
        if(fwrite(&byte, 1, 1, out) != 1) return false;
    }
    return true;
}

bool orbit_packIEEE754(FILE* out, double bits) {
    return orbit_pack64(out, pack754(bits));
}

bool orbit_packBytes(FILE* out, uint8_t* bytes, size_t count) {
    return fwrite(bytes, 1, count, out) == count;
}

bool orbit_unpack8(FILE* in, uint8_t* out) {
    return fread(out, 1, 1, in) == 1;
}

bool orbit_unpack16(FILE* in, uint16_t* out) {
    uint8_t high = 0, low = 0;
    if(fread(&high, 1, 1, in) != 1) return false;
    if(fread(&low, 1, 1, in) != 1) return false;
    
    *out = ((uint16_t)high << 8) | (uint16_t)low;
    return true;
}

bool orbit_unpack32(FILE* in, uint32_t* out) {
    uint8_t byte = 0;
    *out = 0;
    
    for(int8_t i = 3; i >= 0; --i) {
        if(fread(&byte, 1, 1, in) != 1) return false;
        *out = (*out) | ((uint32_t)byte << (8*i));
    }
    return true;
}

bool orbit_unpack64(FILE* in, uint64_t* out) {
    uint8_t byte = 0;
    *out = 0;
    
    for(int8_t i = 7; i >= 0; --i) {
        if(fread(&byte, 1, 1, in) != 1) return false;
        *out = (*out) | ((uint64_t)byte << (8*i));
    }
    return true;
}

bool orbit_unpackIEEE754(FILE* in, double* out) {
    uint64_t raw = 0;
    if(!orbit_unpack64(in, &raw)) return false;
    *out = unpack754(raw);
    return true;
}

bool orbit_unpackBytes(FILE* in, uint8_t* bytes, size_t count) {
    return fread(bytes, 1, count, in) == count;
}
