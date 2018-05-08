//===--------------------------------------------------------------------------------------------===
// orbit/csupport/diag.c - Main implementation of the Orbit diagnostic engine.
// This source is part of Orbit - Compiler Support
//
// Created on 2018-05-08 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdint.h>
#include <orbit/csupport/console.h>
#include <orbit/csupport/diag.h>
#include <orbit/utils/assert.h>

OrbitDiagManager orbit_defaultDiagManager;

void _orbit_defaultDiagConsumer(OCSource* source, OrbitDiag* diagnostic) {
    char printed[256];
    
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
    fprintf(stderr, " in %s [%d:%d]-----\n", source->path, diagnostic->sourceLoc.line, diagnostic->sourceLoc.column);
    
    console_setColor(stderr, CLI_RESET);
    console_printSourceLocLine(stderr, source, diagnostic->sourceLoc);
    console_printCaret(stderr, diagnostic->sourceLoc, CLI_GREEN);
    
    orbit_diagGetFormat(diagnostic, printed, 512);
    fprintf(stderr, "%s\n\n", printed);
}

void orbit_diagManagerInit(OrbitDiagManager* manager, OCSource* source) {
    OASSERT(manager, "Invalid Diagnostics Manager instance");
    manager->source = source;
    manager->consumer = &_orbit_defaultDiagConsumer;
    manager->diagnosticCount = 0;
}

OrbitDiagID orbit_diagNew(OrbitDiagManager* manager, OrbitDiagLevel level, const char* format) {
    OASSERT(manager, "Diagnostics manager does not exist");
    OASSERT(manager->diagnosticCount < ORBIT_DIAG_MAXCOUNT, "Diagnostics overflow");
    
    uint32_t id = manager->diagnosticCount++;
    OrbitDiag* d = &manager->diagnostics[id];
    d->level = level;
    d->format = format;
    d->paramCount = 0;
    d->params = ORBIT_ALLOC_ARRAY(OrbitDiagParam, 5); // TODO: add capacity flag, move to ORCArray?
    return (OrbitDiagID){.manager = manager, .id=id};
}

void orbit_diagAddParam(OrbitDiagID id, OrbitDiagParam param) {
    OASSERT(id.manager, "Diagnostics manager does not exist");
    
    OrbitDiag* d = &id.manager->diagnostics[id.id];
    OASSERT(d->paramCount < 5, "Diagnostics are limited 5 parameters"); // TODO: remove
    
    d->params[d->paramCount] = param;
    d->paramCount += 1;
}

void orbit_diagAddSourceLoc(OrbitDiagID id, OCSourceLoc loc) {
    OASSERT(id.manager, "Diagnostics manager does not exist");
    OrbitDiag* d = &id.manager->diagnostics[id.id];
    d->sourceLoc = loc;
}

void orbit_diagEmitAll(OrbitDiagManager* manager) {
    orbit_diagEmitAbove(manager, ORBIT_DIAGLEVEL_INFO);
}

void orbit_diagEmitAbove(OrbitDiagManager* manager, OrbitDiagLevel level) {
    OASSERT(manager, "Diagnostics manager does not exist");
    // TODO: Sort diagnostics by severity, location?
    
    for(uint32_t i = 0; i < manager->diagnosticCount; ++i) {
        OrbitDiag* d = &manager->diagnostics[i];
        if(d->level < level) { continue; }
        manager->consumer(manager->source, d);
    }
}

