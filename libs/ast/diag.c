//===--------------------------------------------------------------------------------------------===
// orbit/ast/diag.c - Main implementation of the Orbit diagnostic engine.
// This source is part of Orbit - AST
//
// Created on 2018-05-08 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <orbit/ast/diag.h>
#include <orbit/csupport/console.h>
#include <orbit/csupport/source.h>
#include <orbit/utils/memory.h>

#include "diag_private.h"

OrbitDiagManager orbitDefaultDiagManager;

void _orbitDefaultDiagConsumer(OrbitSource* source, OrbitDiag* diagnostic) {
    assert(ORBIT_SLOC_ISVALID(diagnostic->sourceLoc) && "diagnostics should have a source location");
    OrbitPhysSLoc ploc = orbitSourcePhysicalLoc(source, diagnostic->sourceLoc);
    
    // print basic stuff first:
    console_setColor(stderr, CLI_BOLD);
    switch(diagnostic->level) {
        case ORBIT_DIAGLEVEL_INFO:
            fputs("Info", stderr);
            break;
        case ORBIT_DIAGLEVEL_WARN:
            console_setColor(stderr, CLI_YELLOW);
            fputs("Warning", stderr);
            break;
        case ORBIT_DIAGLEVEL_ERROR:
            console_setColor(stderr, CLI_RED);
            fputs("Error", stderr); 
            break;
    }
    console_setColor(stderr, CLI_RESET);
    console_setColor(stderr, CLI_BOLD);
    fprintf(stderr, " in %s @ %"PRIu32":%"PRIu32":\n\n", source->path, ploc.line, ploc.column);
    console_setColor(stderr, CLI_RESET);
    
    console_printSourceLocLine(stderr, source, diagnostic->sourceLoc);

    if(ORBIT_SRANGE_ISVALID(diagnostic->sourceRange)) {
        console_printUnderlines(stderr, source, diagnostic->sourceLoc, diagnostic->sourceRange);
    } else {
        console_printCaret(stderr, source, diagnostic->sourceLoc);
    }
    
    console_setColor(stderr, CLI_MAGENTA);
    char* printed = orbitDiagGetFormat(diagnostic);
    fprintf(stderr, "%s\n", printed);
    console_setColor(stderr, CLI_RESET);
    ORBIT_DEALLOC_ARRAY(printed, char, strlen(printed)+1);
    fputc('\n', stderr);
}

OrbitDiagID orbitDiagError(OrbitDiagManager* manager, OrbitSLoc loc, const char* fmt, int n, ...) {
    OrbitDiagID id = orbitDiagNew(manager, ORBIT_DIAGLEVEL_ERROR, fmt);
    orbitDiagAddSourceLoc(id, loc);
    
    va_list args;
    va_start(args, n);
    for(int i = 0; i < n; ++i) {
        orbitDiagAddParam(id, va_arg(args, OrbitDiagArg));
    }
    va_end(args);
    
    return id;
}

OrbitDiagID orbitDiagWarn(OrbitDiagManager* manager, OrbitSLoc loc, const char* fmt, int n, ...) {
    OrbitDiagID id = orbitDiagNew(manager, ORBIT_DIAGLEVEL_WARN, fmt);
    orbitDiagAddSourceLoc(id, loc);
    
    va_list args;
    va_start(args, n);
    for(int i = 0; i < n; ++i) {
        orbitDiagAddParam(id, va_arg(args, OrbitDiagArg));
    }
    va_end(args);
    
    return id;
}

OrbitDiagID orbitDiagInfo(OrbitDiagManager* manager, OrbitSLoc loc, const char* fmt, int n, ...) {
    OrbitDiagID id = orbitDiagNew(manager, ORBIT_DIAGLEVEL_INFO, fmt);
    orbitDiagAddSourceLoc(id, loc);
    
    va_list args;
    va_start(args, n);
    for(int i = 0; i < n; ++i) {
        orbitDiagAddParam(id, va_arg(args, OrbitDiagArg));
    }
    va_end(args);
    
    return id;
}



void orbitDiagManagerInit(OrbitDiagManager* manager, OrbitSource* source) {
    assert(manager && "Invalid Diagnostics Manager instance");
    manager->source = source;
    manager->consumer = &_orbitDefaultDiagConsumer;
    manager->errorCount = 0;
    manager->diagnosticCount = 0;
    manager->diagnostics = ORBIT_ALLOC_ARRAY(OrbitDiag, ORBIT_DIAG_MAXCOUNT);
}

void orbitDiagManagerDeinit(OrbitDiagManager* manager) {
    assert(manager && "Invalid Diagnostics Manager instance");
    ORBIT_DEALLOC_ARRAY(manager->diagnostics, OrbitDiag, ORBIT_DIAG_MAXCOUNT);
    manager->diagnostics = NULL;
}

OrbitDiagID orbitDiagNew(OrbitDiagManager* manager, OrbitDiagLevel level, const char* format) {
    assert(manager && "Diagnostics manager does not exist");
    assert(manager->diagnosticCount < ORBIT_DIAG_MAXCOUNT && "Diagnostics overflow");
    if(level >= ORBIT_DIAGLEVEL_ERROR) {
        manager->errorCount += 1;
    }
    uint32_t id = manager->diagnosticCount++;
    OrbitDiag* d = &((OrbitDiag*)manager->diagnostics)[id];
    d->sourceLoc = ORBIT_SLOC_INVALID;
    d->sourceRange.start = ORBIT_SLOC_INVALID;
    d->sourceRange.end  = ORBIT_SLOC_INVALID;
    d->level = level;
    d->format = format;
    d->paramCount = 0;
    return (OrbitDiagID){.manager = manager, .id=id};
}

void orbitDiagAddParam(OrbitDiagID id, OrbitDiagArg param) {
    assert(id.manager && "Diagnostics manager does not exist");
    
    OrbitDiag* d = &((OrbitDiag*)id.manager->diagnostics)[id.id];
    assert(d->paramCount < 10 && "Diagnostics are limited 10 parameters");
    
    d->params[d->paramCount] = param;
    d->paramCount += 1;
}

void orbitDiagAddSourceLoc(OrbitDiagID id, OrbitSLoc loc) {
    assert(id.manager && "Diagnostics manager does not exist");
    OrbitDiag* d = &((OrbitDiag*)id.manager->diagnostics)[id.id];
    d->sourceLoc = loc;
}

void orbitDiagAddSourceRange(OrbitDiagID id, OrbitSRange range) {
    assert(id.manager && "Diagnostics manager does not exist");
    OrbitDiag* d = &((OrbitDiag*)id.manager->diagnostics)[id.id];
    d->sourceRange = range;
}

void orbitDiagEmitAll(OrbitDiagManager* manager) {
    orbitDiagEmitAbove(manager, ORBIT_DIAGLEVEL_INFO);
    manager->diagnosticCount = 0;
}

void orbitDiagEmitAbove(OrbitDiagManager* manager, OrbitDiagLevel level) {
    assert(manager && "Diagnostics manager does not exist");
    // TODO: Sort diagnostics by severity, location?
    
    for(uint32_t i = 0; i < manager->diagnosticCount; ++i) {
        OrbitDiag* d = &((OrbitDiag*)manager->diagnostics)[i];
        if(d->level < level) { continue; }
        manager->consumer(manager->source, d);
    }
}


OrbitSLoc orbitDiagGetLoc(OrbitDiag* diag) {
    assert(diag && "Cannot get source location of an invalid diagnostic");
    return diag->sourceLoc;
}

OrbitSRange orbitDiagGetRange(OrbitDiag* diag) {
    assert(diag && "Cannot get source range of an invalid diagnostic");
    return diag->sourceRange;
}

