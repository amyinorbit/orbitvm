//
//  orbit/source/source.h
//  Orbit - Source Handling
//
//  Created by Amy Parent on 2017-10-23.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_source_h
#define orbit_source_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct _OCSource    OCSource;
typedef struct _OCSourceLoc OCSourceLoc;

struct _OCSource {
    const char*     path;
    const char*     bytes;
    uint64_t        length;
};

struct _OCSourceLoc {
    const char*     start;
    uint64_t        line;
    uint64_t        column;

    uint64_t        length;
    uint64_t        displayWidth;
    bool            startOfLine;
};


/// Creates a source handler by opening the file at [path] and reading its bytes.
OCSource source_readFromPath(const char* path);

/// Creates a source handler by reading the bytes of [file].
OCSource source_readFromFile(FILE* file);

/// Deallocates the memory used to store the bytes in [source].
///
/// !warning: any token and source location pointing to this source file
/// will be invalidated.
void source_close(OCSource* source);

#endif /* orbit_source_h_ */
