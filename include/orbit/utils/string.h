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

struct _UTFString {
    ORCObject       super;
    uint64_t        length;
    uint64_t        capacity;
    char            data[ORBIT_FLEXIBLE_ARRAY_MEMB];
};

UTFString* orbit_utfStringNew(const char* cString, size_t length);
UTFString* orbit_utfStringNewWithCapacity(uint64_t capacity);

UTFString* orbit_utfStringAppend(UTFString* string, codepoint_t point);
UTFString* orbit_utfStringCompact(UTFString* string);

#endif /* orbit_utils_string_h */
