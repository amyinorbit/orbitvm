//
//  orbit_string.h
//  OrbitVM
//
//  Created by Cesar Parent on 2017-01-03.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#ifndef orbit_string_h
#define orbit_string_h

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "orbit_utils.h"

typedef struct _String {
    char*       data;
    size_t      length;
    uint32_t    hash;
} String;

// Sets up [string] to point to [text] directly, without copying bytes.
void orbit_stringInitStatic(String* string, const char* text);

// Sets up [string] by copying the bytes from [text].
void orbit_stringInit(String* string, const char* text);

// Frees up memory used by [string].
void orbit_stringDeinit(String* string);

// Returns whether [strA] and [strB] are the same strings. We check for pointer
// equality first to be potentially much faster.
static inline bool orbit_stringEquals(String* strA, String* strB) {
    OASSERT(strA != NULL, "Null instance error");
    OASSERT(strB != NULL, "Null instance error");
    
    return (strA == strB)
        || (strA->data == strB->data)
        || (strA->length == strB->length
            && strA->hash == strB->hash
            && memcmp(strA->data, strB->data, strA->length) == 0);
}

#endif /* orbit_string_h */
