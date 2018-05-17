//===--------------------------------------------------------------------------------------------===
// orbit/ast/diag.c - Main implementation of the Orbit diagnostic engine.
// This source is part of Orbit - AST
//
// Created on 2018-05-08 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <stdint.h>
#include <stdarg.h>
#include <orbit/ast/diag.h>
#include <orbit/csupport/console.h>

#include "diag_private.h"

OrbitDiagManager orbit_defaultDiagManager;

void _orbit_defaultDiagConsumer(OrbitSource* source, OrbitDiag* diagnostic) {
    
    // print basic stuff first:
    console_setColor(stderr, CLI_BOLD);
    fputs("----- ", stderr);
    switch(diagnostic->level) {
        case ORBIT_DIAGLEVEL_INFO:
            fputs("info", stderr);
            break;
        case ORBIT_DIAGLEVEL_WARN:
            console_setColor(stderr, CLI_YELLOW);
            fputs("warning", stderr);
            break;
        case ORBIT_DIAGLEVEL_ERROR:
            console_setColor(stderr, CLI_RED);
            fputs("error", stderr); 
            break;
    }
    console_setColor(stderr, CLI_RESET);
    console_setColor(stderr, CLI_BOLD);
    fprintf(stderr, " in %s -----\n", source->path);
    
    console_setColor(stderr, CLI_RESET);
    if(diagnostic->hasSourceLoc) {
        console_printSourceLocLine(stderr, source, diagnostic->sourceLoc);
        
        if(diagnostic->hasSourceRange)
            console_printUnderlines(stderr, diagnostic->sourceLoc, diagnostic->sourceRange, CLI_GREEN);
        else
            console_printCaret(stderr, diagnostic->sourceLoc, CLI_GREEN);
    }
    
    char* printed = orbit_diagGetFormat(diagnostic);
    fprintf(stderr, "%s\n\n", printed);
    orbit_dealloc(printed);
}

OrbitDiagID orbit_diagEmitError(OrbitSourceLoc loc, const char* format, int count, ...) {
    OrbitDiagID id = orbit_diagNew(
        &orbit_defaultDiagManager,
        ORBIT_DIAGLEVEL_ERROR,
        format
    );
    orbit_diagAddSourceLoc(id, loc);
    
    va_list args;
    va_start(args, count);
    for(int i = 0; i < count; ++i) {
        orbit_diagAddParam(id, va_arg(args, OrbitDiagArg));
    }
    va_end(args);
    
    return id;
}

void orbit_diagManagerInit(OrbitDiagManager* manager, OrbitSource* source) {
    assert(manager && "Invalid Diagnostics Manager instance");
    manager->source = source;
    manager->consumer = &_orbit_defaultDiagConsumer;
    manager->errorCount = 0;
    manager->diagnosticCount = 0;
    manager->diagnostics = calloc(ORBIT_DIAG_MAXCOUNT, sizeof(OrbitDiag));
}

void orbit_diagManagerDeinit(OrbitDiagManager* manager) {
    assert(manager && "Invalid Diagnostics Manager instance");
    free(manager->diagnostics);
    manager->diagnostics = NULL;
}

OrbitDiagID orbit_diagNew(OrbitDiagManager* manager, OrbitDiagLevel level, const char* format) {
    assert(manager && "Diagnostics manager does not exist");
    assert(manager->diagnosticCount < ORBIT_DIAG_MAXCOUNT && "Diagnostics overflow");
    if(level >= ORBIT_DIAGLEVEL_ERROR) {
        manager->errorCount += 1;
    }
    uint32_t id = manager->diagnosticCount++;
    OrbitDiag* d = &((OrbitDiag*)manager->diagnostics)[id];
    d->hasSourceLoc = false;
    d->hasSourceRange = false;
    d->level = level;
    d->format = format;
    d->paramCount = 0;
    return (OrbitDiagID){.manager = manager, .id=id};
}

void orbit_diagAddParam(OrbitDiagID id, OrbitDiagArg param) {
    assert(id.manager && "Diagnostics manager does not exist");
    
    OrbitDiag* d = &((OrbitDiag*)id.manager->diagnostics)[id.id];
    assert(d->paramCount < 10 && "Diagnostics are limited 10 parameters");
    
    d->params[d->paramCount] = param;
    d->paramCount += 1;
}

void orbit_diagAddSourceLoc(OrbitDiagID id, OrbitSourceLoc loc) {
    assert(id.manager && "Diagnostics manager does not exist");
    OrbitDiag* d = &((OrbitDiag*)id.manager->diagnostics)[id.id];
    d->hasSourceLoc = true;
    d->sourceLoc = loc;
}

void orbit_diagAddSourceRange(OrbitDiagID id, OrbitSourceRange range) {
    assert(id.manager && "Diagnostics manager does not exist");
    OrbitDiag* d = &((OrbitDiag*)id.manager->diagnostics)[id.id];
    d->hasSourceRange = true;
    d->sourceRange = range;
}

void orbit_diagEmitAll(OrbitDiagManager* manager) {
    orbit_diagEmitAbove(manager, ORBIT_DIAGLEVEL_INFO);
    manager->diagnosticCount = 0;
}

void orbit_diagEmitAbove(OrbitDiagManager* manager, OrbitDiagLevel level) {
    assert(manager && "Diagnostics manager does not exist");
    // TODO: Sort diagnostics by severity, location?
    
    for(uint32_t i = 0; i < manager->diagnosticCount; ++i) {
        OrbitDiag* d = &((OrbitDiag*)manager->diagnostics)[i];
        if(d->level < level) { continue; }
        manager->consumer(manager->source, d);
    }
}


OrbitSourceLoc orbit_diagGetLoc(OrbitDiag* diag) {
    assert(diag && "Cannot get source location of an invalid diagnostic");
    return diag->sourceLoc;
}

OrbitSourceRange orbit_diagGetRange(OrbitDiag* diag) {
    assert(diag && "Cannot get source range of an invalid diagnostic");
    return diag->sourceRange;
}

