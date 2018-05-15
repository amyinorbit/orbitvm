//===--------------------------------------------------------------------------------------------===
// orbit/bin/compiler.c - Entry point for the Orbit compiler    
// This source is part of Orbit
//
// Created on 2017-03-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <orbit/ast/ast.h>
#include <orbit/csupport/source.h>
#include <orbit/csupport/string.h>
#include <orbit/csupport/diag.h>
#include <orbit/parser/parser.h>
#include <orbit/sema/type.h>
#include <orbit/utils/memory.h>

int main(int argc, const char** args) {
    fprintf(stderr, "orbitc built on %s @ %s\n", __DATE__, __TIME__);
    int result = 0;
    
    if(argc < 2 || argc > 3) {
        fprintf(stderr, "usage: orbitc source_file [-dump-tokens] [-dump-ast]\n");
        return -1;
    }
    
    orbit_stringPoolInit(1024);
    
    OCSource source = source_readFromPath(args[1]);
    if(!source.bytes) {
        fprintf(stderr, "error: cannot open `%s`\n", args[1]);
        return -1;
    }
    
    orbit_diagManagerInit(&orbit_defaultDiagManager, &source);
    
    if(argc == 3 && strcmp(args[2], "-dump-tokens") == 0) {
        orbit_dumpTokens(&source);
        source_close(&source);
        return 0;
    }
    
    AST* ast = ORCRETAIN(orbit_parse(&source));
    sema_runTypeAnalysis(ast);
    orbit_diagEmitAll(&orbit_defaultDiagManager);
    result = orbit_defaultDiagManager.errorCount == 0 ? 0 : -1;
    
    if(argc == 3 && strcmp(args[2], "-dump-ast") == 0) {
        ast_print(stdout, ast);
    }
    
    ORCRELEASE(ast);
    source_close(&source);
    orbit_stringPoolDeinit();
    return result;
}
