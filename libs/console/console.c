//
//  orbit/console/console.c
//  Orbit - Console Support and Utilities
//
//  Created by Amy Parent on 2017-09-28.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
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

void console_printToken(FILE* out, OCToken token, CLIColor color) {
    console_setColor(out, color);
    fprintf(out, "%.*s", (int)token.sourceLoc.length, token.sourceLoc.start);
    console_setColor(out, CLI_RESET);
}

void console_printTokenLine(FILE* out, OCToken token, CLIColor color) {
    console_setColor(out, color);
    const char* line = token.sourceLoc.start;
    
    // Backtrack until the beginning of the line...
    while(*line != '\n'&& line != token.source->bytes) {
        line -= 1;
    }
    line += 1; // otherwise, we won't print anything.
    
    // ...then print the line itself.
    char utf[6];
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
    console_setColor(out, CLI_RESET);
}

void console_printUnderlines(FILE* out, OCSourceLoc loc, CLIColor color) {
    for(uint64_t i = 0; i < loc.column; ++i) {
        fputc(' ', out);
    }
    console_setColor(out, color);
    fputc('^', out);
    for(uint64_t i = 0; i < loc.displayWidth-1; ++i) {
        fputc('~', out);
    }
    console_setColor(out, CLI_RESET);
    fputc('\n', out);
}
