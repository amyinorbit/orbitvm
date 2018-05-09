//===--------------------------------------------------------------------------------------------===
// orbit/csupport/source.c
// This source is part of Orbit - Compiler Support
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdlib.h>
#include <orbit/csupport/source.h>
#include <orbit/utils/memory.h>

static inline OCSourceLoc minLoc(OCSourceLoc a, OCSourceLoc b) {
    if(a.line != b.line) {
        return a.line < b.line ? a : b;
    }
    return a.column < b.column ? a : b;
}

static inline OCSourceLoc maxLoc(OCSourceLoc a, OCSourceLoc b) {
    if(a.line != b.line) {
        return a.line >= b.line ? a : b;
    }
    return a.column >= b.column ? a : b;
}

OCSourceRange source_rangeFromLength(OCSourceLoc start, uint64_t length) {
    OCSourceLoc end = start;
    start.column += length;
    return (OCSourceRange){start, end};
}

OCSourceRange source_rangeUnion(OCSourceRange a, OCSourceRange b) {
    return (OCSourceRange){minLoc(a.start,b.start), maxLoc(a.end,b.end)};
}

/// Creates a source handler by opening the file at [path] and reading its bytes.
OCSource source_readFromPath(const char* path) {
    FILE* f = fopen(path, "r");
    if(!f) {
        return (OCSource) {
            .path = path,
            .length = 0,
            .bytes = NULL
        };
    }
    OCSource source = source_readFromFile(f);
    source.path = path;
    return source;
}

/// Creates a source handler by reading the bytes of [file].
OCSource source_readFromFile(FILE* file) {
    OCSource source;
    
    fseek(file, 0, SEEK_END);
    uint64_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* bytes = orbit_alloc((length+1) * sizeof(char));
    fread(bytes, sizeof(char), length, file);
    bytes[length] = '\0';
    fclose(file);
    
    source.path = "";
    source.bytes = bytes;
    source.length = length;
    
    return source;
}

/// Deallocates the memory used to store the bytes in [source].
void source_close(OCSource* source) {
    free(source->bytes);
    source->bytes = NULL;
    source->path = "";
    source->length = 0;
}
