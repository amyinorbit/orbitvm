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

typedef struct _OrbitSource        OrbitSource;
typedef struct _OrbitSourceLoc     OrbitSourceLoc;
typedef struct _OrbitSourceRange   OrbitSourceRange;

struct _OrbitSource {
    const char*     path;
    char*           bytes;
    uint64_t        length;
};

struct _OrbitSourceLoc {
    // This is replaced by an offset into the source, (safer and seasier to check)
    // const char*     location;
    uint64_t        offset;
    uint32_t        line;
    uint32_t        column;
};

/// A simple type used to denote a range of characters in a source.
struct _OrbitSourceRange {
    const OrbitSourceLoc start;
    const OrbitSourceLoc end;
};

OrbitSourceRange source_rangeFromLength(OrbitSourceLoc start, uint64_t length);
OrbitSourceRange source_rangeUnion(OrbitSourceRange a, OrbitSourceRange b);

/// Creates a source handler by opening the file at [path] and reading its bytes.
OrbitSource source_readFromPath(const char* path);

/// Creates a source handler by reading the bytes of [file].
OrbitSource source_readFromFile(FILE* file);

/// Deallocates the memory used to store the bytes in [source].
///
/// !warning: any token and source location pointing to this source file
/// will be invalidated.
void source_close(OrbitSource* source);

#endif /* orbit_csupport_source_h */
