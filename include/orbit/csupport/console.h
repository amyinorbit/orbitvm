//===--------------------------------------------------------------------------------------------===
// orbit/csupport/console.h
// This source is part of Orbit - Compiler Support
//
// Created on 2017-09-28 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_csupport_console_h
#define orbit_csupport_console_h

#include <stdio.h>
#include <stdint.h>
#include <orbit/csupport/string.h>
#include <orbit/csupport/source.h>
#include <orbit/csupport/tokens.h>

typedef enum {
    CLI_RESET,
    CLI_BOLD,
    CLI_BLACK,
    CLI_RED,
    CLI_GREEN,
    CLI_YELLOW,
    CLI_BLUE,
    CLI_MAGENTA,
    CLI_CYAN,
    CLI_WHITE,
    CLI_BADCOLOR,
} CLIColor;

void console_setColor(FILE* out, CLIColor color);

void console_printToken(FILE* out, OrbitToken token);
void console_printPooledString(FILE* out, OCStringID id);
void console_printTokenLine(FILE* out, OrbitToken token);
void console_printSourceLocLine(FILE* out, const OrbitSource* source, OrbitSLoc loc);
void console_printCaret(FILE* out, const OrbitSource* source, OrbitSLoc loc);
void console_printUnderlines(FILE* out, const OrbitSource* source, OrbitSLoc loc, OrbitSRange range);

#endif /* orbit_csupport_console_h */
