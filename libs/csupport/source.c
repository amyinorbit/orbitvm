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

static inline OrbitSourceLoc minLoc(OrbitSourceLoc a, OrbitSourceLoc b) {
    if(a.line != b.line) {
        return a.line < b.line ? a : b;
    }
    return a.column < b.column ? a : b;
}

static inline OrbitSourceLoc maxLoc(OrbitSourceLoc a, OrbitSourceLoc b) {
    if(a.line != b.line) {
        return a.line >= b.line ? a : b;
    }
    return a.column >= b.column ? a : b;
}

OrbitSourceRange source_rangeFromLength(OrbitSourceLoc start, uint64_t length) {
    OrbitSourceLoc end = start;
    end.column += length;
    return (OrbitSourceRange){.start=start, .end=end};
}

OrbitSourceRange source_rangeUnion(OrbitSourceRange a, OrbitSourceRange b) {
    return (OrbitSourceRange){.start=minLoc(a.start,b.start), .end=maxLoc(a.end,b.end)};
}

/// Creates a source handler by opening the file at [path] and reading its bytes.
OrbitSource source_readFromPath(const char* path) {
    FILE* f = fopen(path, "r");
    if(!f) {
        return (OrbitSource) {
            .path = path,
            .length = 0,
            .bytes = NULL
        };
    }
    OrbitSource source = source_readFromFile(f);
    source.path = path;
    return source;
}

/// Creates a source handler by reading the bytes of [file].
OrbitSource source_readFromFile(FILE* file) {
    OrbitSource source;
    
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
void source_close(OrbitSource* source) {
    free(source->bytes);
    source->bytes = NULL;
    source->path = "";
    source->length = 0;
}
