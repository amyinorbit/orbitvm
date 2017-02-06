//
//  orbit_utf8.h
//  OrbitVM
//
//  Created by Cesar Parent on 2017-02-06.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#ifndef orbit_utf8_h
#define orbit_utf8_h

#include <stdint.h>

// Functions we need to deal with UTF8-encoded strings and files. The goal is
// to have everything in Orbit be unicode-compatible (strings, but also names
// in source files). The characters allowed as start and part of identifiers
// are the same as the ones defined by clang for C/C++ and for Swift.

typedef int32_t codepoint_t;

// Returns how many bytes are required to encode [point], or -1 if the codepoint
// is not a valid one.
int8_t utf8_codepointBytes(codepoint_t point);


#endif /* orbit_utf8_h */
