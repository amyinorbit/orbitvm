//===--------------------------------------------------------------------------------------------===
// orbit/csupport/source.c
// This source is part of Orbit - Compiler Support
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <stdlib.h>
#include <orbit/csupport/source.h>
#include <orbit/utils/platforms.h>
#include <orbit/utils/memory.h>

typedef struct {
    uint32_t count;
    uint32_t capacity;
    uint32_t data[ORBIT_FLEXIBLE_ARRAY_MEMB];
} _OrbitLineMap;

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

OrbitSRange orbitSrangeFromLength(OrbitSLoc start, uint32_t length) {
    assert(ORBIT_SLOC_ISVALID(start) && "ranges should start with a valid location");
    OrbitSRange range;
    range.start = start;
    range.end = ORBIT_SLOC_MAKE(ORBIT_SLOC_OFFSET(start) + length);
    return range;
}

OrbitSRange orbitSrangeUnion(OrbitSRange a, OrbitSRange b) {
    assert(ORBIT_SRANGE_ISVALID(a) && "both ranges in a union must be valid");
    assert(ORBIT_SRANGE_ISVALID(b) && "both ranges in a union must be valid");
    return (OrbitSRange){.start=MIN(a.start, b.start), .end=MAX(a.end, b.end)};
}

bool orbitSrangeContainsLoc(OrbitSRange range, OrbitSLoc loc) {
    if(!ORBIT_SRANGE_ISVALID(range)) { return false; }
    if(!ORBIT_SLOC_ISVALID(loc)) { return false; }
    return ORBIT_SLOC_OFFSET(loc) >= ORBIT_SRANGE_START(range)
        && ORBIT_SLOC_OFFSET(loc) < ORBIT_SRANGE_END(range);
}

/// Creates a source handler by opening the file at [path] and reading its bytes.
bool orbitSourceInitPath(OrbitSource* source, const char* path) {
    assert(source && "Invalid source manager instance passed");
    
    FILE* f = fopen(path, "r");
    if(!f) { return false; }
    if(!orbitSourceInitFile(source, f)) { return false; };
    source->path = path;
    return true;
}

/// Creates a source handler by reading the bytes of [file].
bool orbitSourceInitFile(OrbitSource* source, FILE* file) {
    assert(source && "Invalid source manager instance passed");
    
    fseek(file, 0, SEEK_END);
    uint64_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* bytes = ORBIT_ALLOC_ARRAY(char, length+1);
    fread(bytes, sizeof(char), length, file);
    bytes[length] = '\0';
    fclose(file);
    
    return orbitSourceInitC(source, bytes, length);
}

bool orbitSourceInitC(OrbitSource* source, char* string, uint32_t length) {
    assert(source && "Invalid source manager instance passed");
    source->bytes = string;
    source->length = length;
    
    _OrbitLineMap* map = ORBIT_ALLOC_FLEX(_OrbitLineMap, uint32_t, 100);
    map->count = 1;
    map->capacity = 100;
    map->data[0] = 0;
    source->lineMap = map;
    return true;
}

#define LINEMAP_LAST(map) ((map)->data[(map)->count-1])

static _OrbitLineMap* _orbitLineMapEnsure(_OrbitLineMap* map, uint32_t size) {
    if(size < map->capacity) { return map; }
    uint32_t oldCapacity = map->capacity;
    while(map->capacity < size)
        map->capacity = ORBIT_GROW_CAPACITY(map->capacity);
    return ORBIT_REALLOC_FLEX(map, _OrbitLineMap, uint32_t, oldCapacity, map->capacity);
}

static uint32_t _orbitInsertLines(const OrbitSource* source, OrbitSLoc loc) {
    // Insert any newline we find up to [loc]
    assert(source && "invalid source passed");
    assert(ORBIT_SLOC_OFFSET(loc) < source->length && "location is out of range for this source");
    
    _OrbitLineMap* map = (_OrbitLineMap*)source->lineMap;
    
    uint32_t offset = LINEMAP_LAST(map);
    uint32_t line = map->count - 1;
    
    bool stopAtNext = false;
    while(offset <= source->length) {
        if(offset == ORBIT_SLOC_OFFSET(loc)) { stopAtNext = true; }
        char c = source->bytes[offset++];
        if(c != '\n' || c == '\0') { continue; }
        map = _orbitLineMapEnsure(map, map->count+1);
        map->data[map->count] = offset;
        map->count += 1;
        
        if(stopAtNext) { break; }
        line += 1;
    }
    ((OrbitSource*)source)->lineMap = map;
    return line;
}

uint32_t _orbitLineMapSearch(_OrbitLineMap* map, OrbitSLoc loc, bool* found) {
    *found = false;
    for(uint32_t line = 0; line < map->count-1; ++line) {
        uint32_t offset = ORBIT_SLOC_OFFSET(loc);
        if(offset < map->data[line] || offset >= map->data[line+1]) { continue; }
        *found = true;
        return line;
    }
    return 0;
}

OrbitPhysSLoc orbitSourcePhysicalLoc(const OrbitSource* source, OrbitSLoc loc) {
    assert(source && "invalid source passed");
    assert(ORBIT_SLOC_ISVALID(loc) && "invalid source locations have no physical locations");

    _OrbitLineMap* map = (_OrbitLineMap*)source->lineMap;
    OrbitPhysSLoc ploc;
    uint32_t offset = ORBIT_SLOC_OFFSET(loc);
    
    // Slow path, we need to insert lines
    if(offset >= LINEMAP_LAST(map)) {
        ploc.line = _orbitInsertLines(source, loc)+1;
        ploc.column = 1 + (offset - map->data[ploc.line-1]);
        return ploc;
    }
    
    bool found = false;
    ploc.line = _orbitLineMapSearch(map, loc, &found)+1;
    assert(found && "source location line not found");
    ploc.column = 1 + (offset - map->data[ploc.line-1]);
    return ploc;
}

/// Deallocates the memory used to store the bytes in [source].
void orbitSourceDeinit(OrbitSource* source) {
    source->bytes = ORBIT_DEALLOC_ARRAY(source->bytes, char, (source->length+1));
    source->path = "";
    _OrbitLineMap* map = (_OrbitLineMap*)source->lineMap;
    ORBIT_DEALLOC_FLEX(map, _OrbitLineMap, uint32_t, map->capacity);
}
