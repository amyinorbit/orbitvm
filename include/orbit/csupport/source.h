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

typedef struct _OrbitSource     OrbitSource;
typedef uint32_t                OrbitSLoc;
typedef struct _OrbitSRange     OrbitSRange;
typedef struct _OrbitPhysSLoc   OrbitPhysSLoc;

struct _OrbitSource {
    const char*     path;
    char*           bytes;
    uint64_t        length;
    void*           lineMap;
};

// Source locations need to be as small as possible so we can pass them around by value
// and embed them where needed -- AST nodes, diagnostics, etc. We don't need line and
// column numbers in most places, until we have to print, so we don't embed those in
// the location: we'll use the source manager object instead (probably using a table)
// to fetch those.
//
// For the moment, SLoc is 32 bytes (31 bytes for the offset), which means we can't
// address source files bigger than 2GB, which seems sensible but might become a problem.
// Maybe we need to have some table structure mapping to bigger addresses.
struct _OrbitSLoc {
    uint32_t    offset:31;
    uint32_t    valid:1;
};

// Physical locations are fetched from the location table in a source object. The column
// field is the 'physical' column, in terms of bytes since the start of the line (which is
// what most editors display), but the source object can be used to fetch the display column
// to handle pretty-printed diagnostic underlines.
struct _OrbitPhysSLoc {
    uint32_t    line;
    uint32_t    column;
};

/// A simple type used to denote a range of characters in a source.
struct _OrbitSRange {
    OrbitSLoc start;
    OrbitSLoc end;
};

#define ORBIT_SOURCE_LOC(offset) ((OrbitSLoc){.offset=(offset), .valid=1})

#define ORBIT_SLOC_MAKE(offset)     ((offset) & 0x7fffffff)
#define ORBIT_SLOC_INVALID          (0x80000000)
#define ORBIT_SLOC_EQUAL(a,b)       ((a) == (b))

#define ORBIT_SLOC_ISVALID(loc)     (((loc) >> 31) == 0)
#define ORBIT_SLOC_OFFSET(loc)      (loc & 0x7fffffff)
#define ORBIT_SRANGE_ISVALID(range) (ORBIT_SLOC_ISVALID((range).start) && ORBIT_SLOC_ISVALID((range).end))

#define ORBIT_SRANGE_START(range)   (ORBIT_SLOC_OFFSET((range).start))
#define ORBIT_SRANGE_END(range)     (ORBIT_SLOC_OFFSET((range).end))

OrbitSRange orbit_srangeFromLength(OrbitSLoc start, uint32_t length);
OrbitSRange orbit_srangeUnion(OrbitSRange a, OrbitSRange b);
bool orbit_srangeContainsLoc(OrbitSRange range, OrbitSLoc sloc);

/// Creates a source handler by opening the file at [path] and reading its bytes.
bool orbit_sourceInitPath(OrbitSource* source, const char* path);

/// Creates a source handler by reading the bytes of [file].
bool orbit_sourceInitFile(OrbitSource* source, FILE* file);

OrbitPhysSLoc orbit_sourcePhysicalLoc(const OrbitSource* source, OrbitSLoc loc);

/// Deallocates the memory used to store the bytes in [source].
///
/// !warning: any token and source location pointing to this source file
/// will be invalidated.
void orbit_sourceDeinit(OrbitSource* source);

#endif /* orbit_csupport_source_h */
