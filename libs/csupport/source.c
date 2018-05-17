//===--------------------------------------------------------------------------------------------===
// orbit/csupport/source.c
// This source is part of Orbit - Compiler Support
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
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

#define S_MAX(a, b) ((a).offset > (b).offset ? (a) : (b))
#define S_MIN(a, b) ((a).offset < (b).offset ? (a) : (b))

OrbitSRange orbit_srangeFromLength(OrbitSLoc start, uint32_t length) {
    OrbitSRange range;
    range.start = start;
    range.end.valid = 1;
    range.end.offset = start.offset+length;
    return range;
}

OrbitSRange orbit_srangeUnion(OrbitSRange a, OrbitSRange b) {
    return (OrbitSRange){.start=S_MIN(a.start, b.start), .end=S_MAX(a.end, b.end)};
}

/// Creates a source handler by opening the file at [path] and reading its bytes.
bool orbit_sourceInitPath(OrbitSource* source, const char* path) {
    assert(source && "Invalid source manager instance passed");
    
    FILE* f = fopen(path, "r");
    if(!f) { return false; }
    if(!orbit_sourceInitFile(source, f)) { return false; };
    source->path = path;
    return true;
}

/// Creates a source handler by reading the bytes of [file].
bool orbit_sourceInitFile(OrbitSource* source, FILE* file) {
    assert(source && "Invalid source manager instance passed");
    
    fseek(file, 0, SEEK_END);
    uint64_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* bytes = orbit_alloc((length+1) * sizeof(char));
    fread(bytes, sizeof(char), length, file);
    bytes[length] = '\0';
    fclose(file);
    
    source->path = "";
    source->bytes = bytes;
    source->length = length;
    
    _OrbitLineMap* map = ORBIT_ALLOC_FLEX(_OrbitLineMap, uint32_t, 100);
    map->count = 1;
    map->capacity = 100;
    map->data[0] = 0;
    source->lineMap = map;
    
    return true;
}

#define LINEMAP_LAST(map) ((map)->data[(map)->count-1])

static _OrbitLineMap* _orbit_lineMapEnsure(_OrbitLineMap* map, uint32_t size) {
    if(size < map->capacity) { return map; }
    while(map->capacity < size) {
        map->capacity *= 2;
    }
    return ORBIT_REALLOC_FLEX(map, _OrbitLineMap, uint32_t, map->capacity);
}

static void _orbit_dumpLineMap(_OrbitLineMap* map) {
    printf("--LINE MAP--\n");
    for(uint32_t i = 0; i < map->count; ++i) {
        printf("  %03u -> %u\n", i+1, map->data[i]);
    }
}

static uint32_t _orbit_insertLines(const OrbitSource* source, OrbitSLoc loc) {
    // Insert any newline we find up to [loc]
    assert(source && "invalid source passed");
    assert(loc.offset < source->length && "location is out of range for this source");
    
    _OrbitLineMap* map = (_OrbitLineMap*)source->lineMap;
    
    uint32_t offset = LINEMAP_LAST(map);
    uint32_t line = map->count - 1;
    
    bool stopAtNext = false;
    while(offset <= source->length) {
        if(offset == loc.offset) {
            stopAtNext = true;
        }
        char c = source->bytes[offset++];
        if(c != '\n' || c == '\0') { continue; }
        map = _orbit_lineMapEnsure(map, map->count+1);
        map->data[map->count] = offset;
        map->count += 1;
        
        if(stopAtNext) {
            break;
        }
        line += 1;
    }
    ((OrbitSource*)source)->lineMap = map;
    return line;
}

uint32_t _orbit_lineMapSearch(_OrbitLineMap* map, OrbitSLoc loc, bool* found) {
    *found = false;
    for(uint32_t line = 0; line < map->count-1; ++line) {
        if(loc.offset <= map->data[line] || loc.offset > map->data[line+1]) { continue; }
        *found = true;
        return line;
    }
    return 0;
}

OrbitPhysSLoc orbit_sourcePhysicalLoc(const OrbitSource* source, OrbitSLoc loc) {
    assert(source && "invalid source passed");
    assert(loc.valid && "invalid source locations have no physical locations");

    _OrbitLineMap* map = (_OrbitLineMap*)source->lineMap;
    OrbitPhysSLoc ploc;
    //_orbit_dumpLineMap(map);
    
    // Slow path, we need to insert lines
    if(loc.offset > LINEMAP_LAST(map)) {
        ploc.line = _orbit_insertLines(source, loc)+1;
        ploc.column = loc.offset - map->data[ploc.line-1];
        return ploc;
    }
    
    bool found = false;
    ploc.line = _orbit_lineMapSearch(map, loc, &found)+1;
    assert(found && "source location line not found");
    ploc.column = loc.offset - map->data[ploc.line-1];
    return ploc;
}

/// Deallocates the memory used to store the bytes in [source].
void orbit_sourceDeinit(OrbitSource* source) {
    free(source->bytes);
    source->bytes = NULL;
    source->path = "";
    source->length = 0;
}
