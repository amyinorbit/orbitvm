//===--------------------------------------------------------------------------------------------===
// orbit/bin/compiler.c - Entry point for the Orbit compiler    
// This source is part of Orbit
//
// Created on 2017-03-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <orbit/ast/ast.h>
#include <orbit/ast/context.h>
#include <orbit/ast/diag.h>
#include <orbit/csupport/source.h>
#include <orbit/csupport/string.h>
#include <orbit/parser/parser.h>
#include <orbit/sema/typecheck.h>
#include <orbit/utils/memory.h>

int main(int argc, const char** args) {
    fprintf(stderr, "orbitc built on %s @ %s by Amy Parent\n", __DATE__, __TIME__);
    int result = 0;
    
    if(argc < 2 || argc > 3) {
        fprintf(stderr, "usage: orbitc source_file [-dump-tokens] [-dump-ast]\n");
        return -1;
    }
    
    orbit_stringPoolInit(1024);
    
    OrbitASTContext ctx;
    orbit_astContextInit(&ctx);
    
    if(!orbit_sourceInitPath(&ctx.source, args[1])) {
        fprintf(stderr, "error: cannot open `%s`\n", args[1]);
        return -1;
    }
    
    if(argc == 3 && strcmp(args[2], "-dump-tokens") == 0) {
        orbit_dumpTokens(&ctx);
    } else {
        orbit_parse(&ctx);
        
        //OrbitAST* ast = ORCRETAIN(orbit_parse(&source));
        sema_runTypeAnalysis(&ctx);
        orbit_diagEmitAll(&ctx.diagnostics);
        result = ctx.diagnostics.errorCount == 0 ? 0 : -1;
        
        if(argc == 3 && strcmp(args[2], "-dump-ast") == 0) {
            orbit_astPrint(stdout, ctx.root);
        }
    }
    
    orbit_astContextDeinit(&ctx);
    orbit_stringPoolDeinit();
    return result;
}
