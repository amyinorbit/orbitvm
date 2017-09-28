//
//  orbit/console/console.c
//  Orbit - Console Support and Utilities
//
//  Created by Amy Parent on 2017-09-28.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <orbit/console/console.h>

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

void console_printUnderlines(FILE* out, uint64_t column, uint64_t length) {
    for(uint64_t i = 0; i < column; ++i) {
        fputc(' ', out);
    }
    console_setColor(out, CLI_GREEN);
    fputc('^', out);
    for(uint64_t i = 0; i < length-1; ++i) {
        fputc('~', out);
    }
    console_setColor(out, CLI_RESET);
    fputc('\n', out);
}
