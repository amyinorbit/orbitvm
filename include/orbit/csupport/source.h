//===--------------------------------------------------------------------------------------------===
// orbit/csupport/source.h
// This source is part of Orbit - Compiler Support
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_csupport_source_h
#define orbit_csupport_source_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct _OCSource    OCSource;
typedef struct _OCSourceLoc OCSourceLoc;

struct _OCSource {
    const char*     path;
    char*           bytes;
    uint64_t        length;
};

struct _OCSourceLoc {
    // This is replaced by an offset into the source, (safer and seasier to check)
    // const char*     location;
    uint64_t        offset;
    uint32_t        line;
    uint32_t        column;
};

/// A simple type used to denote a range of characters in a source.
struct _OCSourceRange {
    const OCSourceLoc start;
    const OCSourceLoc end;
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

#endif /* orbit_csupport_source_h */
