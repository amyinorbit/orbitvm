//===--------------------------------------------------------------------------------------------===
// orbit/csupport/console.c
// This source is part of Orbit - Compiler Support
//
// Created on 2017-09-28 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <orbit/csupport/console.h>
#include <term/colors.h>
#include <unic/unic.h>

void console_printToken(FILE* out, OrbitToken token) {
    const char* bytes = token.source->bytes + ORBIT_SLOC_OFFSET(token.sourceLoc);
    fprintf(out, "%.*s", (int)token.length, bytes);
}

void console_printPooledString(FILE* out, OCStringID id) {
    OCString* str = orbitStringPoolGet(id);
    if(!str) { return; }
    fprintf(out, "%.*s", (int)str->length, str->data);
}

void console_printSourceLocLine(FILE* out, const OrbitSource* source, OrbitSLoc loc) {
    const char* line = source->bytes + ORBIT_SLOC_OFFSET(loc);
    // Backtrack until the beginning of the line...
    while(*(line-1) != '\n'&& line != source->bytes) {
        line -= 1;
    }
    
    // ...then print the line itself.
    char utf[6];
    OrbitPhysSLoc ploc = orbitSourcePhysicalLoc(source, loc);
    fprintf(out, "%"PRIu32"|", ploc.line);
    while(line < source->bytes + source->length) {
        uint64_t remaining = (source->bytes + source->length) - line;
        uint8_t size = 0;
        UnicodeScalar c = unic_utf8Read(line, remaining, &size);
        if(c == '\0' || c == '\n') { break; }
        size = utf8_writeCodepoint(c, utf, 6);
        line += size;
        utf[size] = '\0';
        fprintf(out, "%.*s", size, utf);
    }
    fprintf(out, "\n");
}

void console_printTokenLine(FILE* out, OrbitToken token) {
    console_printSourceLocLine(out, token.source, token.sourceLoc);
}

void console_printCaret(FILE* out, const OrbitSource* source, OrbitSLoc loc) {
    
    OrbitPhysSLoc ploc = orbitSourcePhysicalLoc(source, loc);
    uint8_t offset = 2 + floor (log10 (ploc.line));
    for(uint64_t i = 1; i < ploc.column + offset; ++i) {
        fputc(' ', out);
    }
    termColorFG(out, kTermGreen);
    fputc('^', out);
    termReset(out);
    fputc('\n', out);
}

void console_printUnderlines(FILE* out, const OrbitSource* source, OrbitSLoc loc, OrbitSRange range) {
    assert(orbitSrangeContainsLoc(range, loc) && "caret must be in the source range");
    
    OrbitPhysSLoc start = orbitSourcePhysicalLoc(source, range.start);
    uint32_t end = start.column + (ORBIT_SRANGE_END(range) - ORBIT_SRANGE_START(range));
    uint32_t caret = start.column + (ORBIT_SLOC_OFFSET(loc) - ORBIT_SRANGE_START(range));
    
    uint8_t offset = 2 + floor (log10 (start.line));
    for(uint64_t i = 1; i < start.column + offset; ++i) {
        fputc(' ', out);
    }
    termColorFG(out, kTermGreen);
    for(uint64_t i = start.column; i < end; ++i) {
        fputc(i == caret ? '^' : '~', out);
    }
    termReset(out);
    fputc('\n', out);
}
