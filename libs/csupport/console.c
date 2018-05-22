//===--------------------------------------------------------------------------------------------===
// orbit/csupport/console.c
// This source is part of Orbit - Compiler Support
//
// Created on 2017-09-28 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <orbit/csupport/console.h>
#include <orbit/utils/utf8.h>

static const char* _ansiCodes[] = {
    [CLI_RESET] = "\033[0m",
    [CLI_BOLD] = "\033[1m",
    [CLI_RED] = "\033[31m",
    [CLI_GREEN] = "\033[32m",
    [CLI_YELLOW] = "\033[33m",
    [CLI_BLUE] = "\033[34m",
    [CLI_MAGENTA] = "\033[35m",
    [CLI_CYAN] = "\033[36m",
    [CLI_BADCOLOR]  = "",
};

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)) || defined (__MINGW32__)
#include <unistd.h>
#define orbit_consoleSupportsColor(file) (isatty(fileno(file)))
#else
#define orbit_consoleSupportsColor(file) (false)
#endif


void console_setColor(FILE* out, CLIColor color) {
    if(!orbit_consoleSupportsColor(out)) { return; }
    if(color > CLI_BADCOLOR) { return; }
    fprintf(out, "%s", _ansiCodes[color]);
}

void console_printToken(FILE* out, OrbitToken token) {
    const char* bytes = token.source->bytes + token.sourceLoc.offset;
    fprintf(out, "%.*s", (int)token.length, bytes);
}

void console_printPooledString(FILE* out, OCStringID id) {
    OCString* str = orbit_stringPoolGet(id);
    if(!str) { return; }
    fprintf(out, "%.*s", (int)str->length, str->data);
}

void console_printSourceLocLine(FILE* out, const OrbitSource* source, OrbitSLoc loc) {
    const char* line = source->bytes + loc.offset;
    // Backtrack until the beginning of the line...
    while(*(line-1) != '\n'&& line != source->bytes) {
        line -= 1;
    }
    
    // ...then print the line itself.
    char utf[6];
    OrbitPhysSLoc ploc = orbit_sourcePhysicalLoc(source, loc);
    fprintf(out, "%"PRIu32"|", ploc.line);
    while(line < source->bytes + source->length) {
        uint64_t remaining = (source->bytes + source->length) - line;
        codepoint_t c = utf8_getCodepoint(line, remaining);
        if(c == '\0' || c == '\n') { break; }
        int size = utf8_writeCodepoint(c, utf, 6);
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
    
    OrbitPhysSLoc ploc = orbit_sourcePhysicalLoc(source, loc);
    uint8_t offset = 2 + floor (log10 (ploc.line));
    for(uint64_t i = 1; i < ploc.column + offset; ++i) {
        fputc(' ', out);
    }
    console_setColor(out, CLI_GREEN);
    fputc('^', out);
    console_setColor(out, CLI_RESET);
    fputc('\n', out);
}

void console_printUnderlines(FILE* out, const OrbitSource* source, OrbitSLoc loc, OrbitSRange range) {
    assert(loc.offset >= range.start.offset && loc.offset < range.end.offset);
    
    OrbitPhysSLoc start = orbit_sourcePhysicalLoc(source, range.start);
    uint32_t end = start.column + (range.end.offset - range.start.offset);
    uint32_t caret = start.column + (loc.offset - range.start.offset);
    
    uint8_t offset = 2 + floor (log10 (start.line));
    for(uint64_t i = 1; i < start.column + offset; ++i) {
        fputc(' ', out);
    }
    console_setColor(out, CLI_GREEN);
    for(uint64_t i = start.column; i < end; ++i) {
        fputc(i == caret ? '^' : '~', out);
    }
    console_setColor(out, CLI_RESET);
    fputc('\n', out);
}
