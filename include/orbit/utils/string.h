//
//  orbit/utils/string.h
//  Orbit - Utils
//
//  Created by Amy Parent on 2017-11-21.
//  Copyright © 2016-2017 Amy Parent. All rights reserved.
//
#ifndef orbit_utils_string_h
#define orbit_utils_string_h

#include <stdint.h>
#include <orbit/utils/utf8.h>
#include <orbit/utils/platforms.h>
#include <orbit/utils/memory.h>

typedef struct _UTFString UTFString;
typedef struct _UTFConstString UTFConstString;

// TODO: replace with const UTFString* ?
struct _UTFConstString {
    ORCObject       super;
    const uint64_t  length;
    const uint32_t  hash;
    const char      data[ORBIT_FLEXIBLE_ARRAY_MEMB];
};

struct _UTFString {
    ORCObject       super;
    uint64_t        length;
    uint64_t        capacity;
    char            data[ORBIT_FLEXIBLE_ARRAY_MEMB];
};

// Computes the FNV-1a hash of [string].
// This is O(n) complexity and should be used lightly.
uint32_t orbit_hashString(const char* string, size_t length);

UTFString* orbit_utfStringNew(const char* cString, size_t length);
UTFString* orbit_utfStringNewWithCapacity(uint64_t capacity);

UTFString* orbit_utfStringAppend(UTFString* string, codepoint_t point);
UTFConstString* orbit_utfStringConstCopy(UTFString* string);

#endif /* orbit_utils_string_h */
