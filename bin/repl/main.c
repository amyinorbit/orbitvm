//===--------------------------------------------------------------------------------------------===
// main.c - Entry point for the Orbit Runtime 2.0 REPL
// This source is part of Orbit
//
// Created on 2019-06-27 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <orbit/rt2/chunk.h>
#include <orbit/rt2/vm.h>
#include <orbit/rt2/debug.h>
#include <orbit/rt2/opcodes.h>
#include <orbit/rt2/value_object.h>
#include <orbit/rt2/invocation.h>

#include <orbit/ast/context.h>
#include <orbit/ast/diag.h>
#include <orbit/ast/traversal.h>
#include <orbit/csupport/console.h>
#include <orbit/csupport/tokens.h>
#include <orbit/csupport/string.h>
#include <orbit/utils/memory.h>
#include <orbit/parser/parser.h>
#include <orbit/sema/typecheck.h>
#include <orbit/codegen/codegen.h>

typedef struct {
    OrbitGC* gc;
    OrbitFunction* fn;
} Compiler;

OrbitResult repl_compile(Compiler comp, int line, const char* input) {
    size_t length = strlen(input);
    char* src = orbit_allocator(NULL, 0, length + 1);
    memcpy(src, input, length);
    src[length] = '\0';
    
    OrbitASTContext ctx;
    orbit_astContextInit(&ctx);
    ctx.source.path = "repl.orbit";
    orbit_sourceInitC(&ctx.source, src, length);
    orbit_parse(&ctx);

    orbit_semaCheck(&ctx);
    orbit_diagEmitAll(&ctx.diagnostics);
    orbit_astPrint(stdout, ctx.root);
    if(ctx.diagnostics.errorCount) return ORBIT_COMPILE_ERROR;

    orbit_codegen(comp.gc, comp.fn, &ctx);
    // orbit_debugFunction(comp.fn, "repl");
    orbit_functionWrite(comp.gc, comp.fn, OP_return, 1);

    orbit_astContextDeinit(&ctx);
    return ORBIT_OK;
}


void repl(OrbitVM* vm) {
    
    printf("Orbit 2019.6 ** REPL\n");
    int lineNumber = 1;
    char line[1024];
    for(;;) {

        console_setColor(stdout, CLI_BLACK);
        printf("%3d> ", lineNumber);
        console_setColor(stdout, CLI_RESET);
        
        if(!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;                             
        }
        
        OrbitFunction* fn = orbit_functionNew(&vm->gc);
        orbit_gcPush(&vm->gc, (OrbitObject*)fn);
        Compiler comp = (Compiler){&vm->gc, fn};
        
        if(repl_compile(comp, lineNumber, line) == ORBIT_OK) {
            orbit_run(vm, fn);
            console_setColor(stderr, CLI_CYAN);
            orbit_debugTOS(vm);
            // orbit_debugStack(vm);
            console_setColor(stderr, CLI_RESET);
            orbit_gcRun(&vm->gc);
        }
        
        orbit_gcPop(&vm->gc);
        orbit_gcRun(&vm->gc);
        lineNumber += 1;
    }
}

OrbitResult compileFile(OrbitVM* vm, const char* path) {
    OrbitFunction* fn = orbit_functionNew(&vm->gc);
    orbit_gcPush(&vm->gc, (OrbitObject*)fn);
    Compiler comp = (Compiler){&vm->gc, fn};
    
    OrbitASTContext ctx;
    orbit_astContextInit(&ctx);
    if(!orbit_sourceInitPath(&ctx.source, path))
        fprintf(stderr, "error: cannot open source file '%s'\n", path);
    orbit_parse(&ctx);
    
    orbit_semaCheck(&ctx);
    orbit_diagEmitAll(&ctx.diagnostics);
    orbit_astPrint(stdout, ctx.root);
    if(ctx.diagnostics.errorCount) return ORBIT_COMPILE_ERROR;
    
    orbit_codegen(comp.gc, comp.fn, &ctx);
    orbit_debugFunction(fn, path);
    orbit_functionWrite(&vm->gc, fn, OP_return, 1);
    
    orbit_astContextDeinit(&ctx);
    return ORBIT_OK;
}

int main(int argc, const char** argv) {
    setlocale(LC_ALL, NULL);
    orbit_stringPoolInit(1024);

    OrbitVM vm;
    orbit_vmInit(&vm);
    
    if(argc <= 1)
        repl(&vm);
    else
        compileFile(&vm, argv[1]);
    
    orbit_vmDeinit(&vm);
    orbit_stringPoolDeinit();
}