//===--------------------------------------------------------------------------------------------===
// orbit/console/console.c
// This source is part of Orbit - Console Support and Utilities
//
// Created on 2017-09-28 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <inttypes.h>
#include <orbit/console/console.h>
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

void console_setColor(FILE* out, CLIColor color) {
    // TODO: implement windows equivalent
    if(color > CLI_BADCOLOR) { return; }
    fprintf(out, "%s", _ansiCodes[color]);
}

void console_printToken(FILE* out, OCToken token) {
    const char* bytes = token.source->bytes + token.sourceLoc.offset;
    fprintf(out, "%.*s", (int)token.length, bytes);
}

void console_printPooledString(FILE* out, OCStringID id) {
    OCString* str = orbit_stringPoolGet(id);
    if(!str) { return; }
    fprintf(out, "%.*s", (int)str->length, str->data);
}

void console_printTokenLine(FILE* out, OCToken token) {
    const char* line = token.source->bytes + token.sourceLoc.offset;
    
    // Backtrack until the beginning of the line...
    while(*line != '\n'&& line != token.source->bytes) {
        line -= 1;
    }
    line += 1; // otherwise, we won't print anything.
    
    // ...then print the line itself.
    char utf[6];
    fprintf(out, "%"PRIu32"|", token.sourceLoc.line);
    while(line < token.source->bytes + token.source->length) {
        uint64_t remaining = (token.source->bytes + token.source->length) - line;
        codepoint_t c = utf8_getCodepoint(line, remaining);
        if(c == '\0' || c == '\n') { break; }
        int size = utf8_writeCodepoint(c, utf, 6);
        line += size;
        utf[size] = '\0';
        fprintf(out, "%.*s", size, utf);
    }
    fprintf(out, "\n");
}

void console_printUnderlines(FILE* out, OCToken tok, CLIColor color) {
    uint8_t offset = 2 + tok.sourceLoc.line / 10;
    for(uint64_t i = 0; i < tok.sourceLoc.column + offset; ++i) {
        fputc(' ', out);
    }
    console_setColor(out, color);
    fputc('^', out);
    for(uint64_t i = 0; i < tok.displayLength-1; ++i) {
        fputc('~', out);
    }
    console_setColor(out, CLI_RESET);
    fputc('\n', out);
}
