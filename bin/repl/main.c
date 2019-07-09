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
#include <assert.h>

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

#include <term/arg.h>

typedef struct {
    OrbitGC* gc;
    OrbitFunction* fn;
} Compiler;

typedef struct {
    bool dumpAST;
    bool dumpBytecode;
    bool codegen;
    bool run;
    const char* input;
} Options;

OrbitResult repl_compile(Compiler comp, int line, const char* input, Options options) {
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
    if(options.dumpAST) orbit_astPrint(stdout, ctx.root);
    if(ctx.diagnostics.errorCount) return ORBIT_COMPILE_ERROR;

    orbit_codegen(comp.gc, comp.fn, &ctx);
    orbit_functionWrite(comp.gc, comp.fn, OP_return_repl, 1);
    if(options.dumpBytecode) orbit_debugFunction(comp.fn, "repl");

    orbit_astContextDeinit(&ctx);
    return ORBIT_OK;
}


void repl(OrbitVM* vm, Options options) {
    
    printf("Welcome to Orbit version 2019.6 repl (" __DATE__ ")\n");
    printf("[built with " __COMPILER_NAME__ "]\n");
    int lineNumber = 1;
    char line[1024];
    for(;;) {

        console_setColor(stdout, CLI_CYAN);
        printf("orbit:%3d> ", lineNumber);
        console_setColor(stdout, CLI_RESET);
        
        if(!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;                             
        }
        
        OrbitFunction* fn = orbit_functionNew(&vm->gc);
        orbit_gcPush(&vm->gc, (OrbitObject*)fn);
        Compiler comp = (Compiler){&vm->gc, fn};
        
        if(repl_compile(comp, lineNumber, line, options) == ORBIT_OK) {
            orbit_run(vm, fn);
            console_setColor(stderr, CLI_GREEN);
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

OrbitResult compileFile(OrbitVM* vm, const char* path, Options options) {
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
    if(options.dumpAST) orbit_astPrint(stdout, ctx.root);
    if(ctx.diagnostics.errorCount) return ORBIT_COMPILE_ERROR;
    
    if(!options.codegen) goto cleanup; 
    
    orbit_codegen(comp.gc, comp.fn, &ctx);
    if(options.dumpBytecode) orbit_debugFunction(fn, path);
    orbit_functionWrite(&vm->gc, fn, OP_return, 1);
    
    if(!options.run) goto cleanup;
    
    orbit_run(vm, fn);
    
cleanup:
    orbit_astContextDeinit(&ctx);
    orbit_gcRun(&vm->gc);
    return ORBIT_OK;
}

static void printVersion() {
    printf("Orbit version 2019.6 repl (" __DATE__ ")\n");
    printf("built with " __COMPILER_NAME__ "\n");
    printf("Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>\n");
}

static _Noreturn void printHelp(TUArgParser* parser) {
    printVersion();
    printf("\nUsage: orbit [option]... source_file\n");
    printf("  or   orbit [option]...\n\n");
    termPrintHelp(stdout, parser);
    exit(1);
}

/*
typedef struct {
    bool dumpAST;
    bool dumpBytecode;
    bool codegen;
    bool run;
} Options;
*/

static void setOptions(Options* options, TUArg* args, int count) {
    for(int i = 0; i < count; ++i) {
        switch(args[i].name) {
        case 'v':
            options->dumpAST = true;
            options->dumpBytecode = true;
            break;
        case 'n':
            options->codegen = false;
            break;
        case 's':
            options->dumpAST = true;
            break;
        case 'x':
            options->dumpBytecode = true;
            break;
        case '\0':
            if(args[i].kind == TU_ARG_POS) {
                options->input = args[i].as.value;
            }
            break;
        }
        
    }
}

static void parseArguments(Options* options, int argc, const char** argv) {
    
    TUArgParser parser;
    termArgParserInit(&parser, argc, argv);
    
    termArgAddOption(&parser, 'v', "debug-vm", "print debug information (like -x -s)");
    termArgAddOption(&parser, 'n', "syntax-only", "stop compilation before generating code");
    termArgAddOption(&parser, 's', "print-ast", "print syntax tree before running");
    termArgAddOption(&parser, 'x', "print-bytecode", "print compiled bytecode before running");
    
    TUArg args[32];
    int result = termArgParse(&parser, args, 32);
    
    switch(result) {
    case TU_ARGRESULT_HELP:
        printHelp(&parser);
        break;
        
    case TU_ARGRESULT_VERSION:
        printVersion();
        exit(0);
        break;
        
    case TU_ARGRESULT_ERROR:
        fprintf(stderr, "orbit: error: %s\n", parser.error);
        exit(1);
        break;
        
    case TU_ARGRESULT_NOMEM:
        assert(false && "not enough arg slots");
        break;
        
    default:
        setOptions(options, args, result);
        break;
    }
}

int main(int argc, const char** argv) {
    setlocale(LC_ALL, NULL);
    orbit_stringPoolInit(1024);

    OrbitVM vm;
    orbit_vmInit(&vm);
    
    // const char* inputFile = NULL;
    Options options = (Options){false, false, true, true, NULL};
    parseArguments(&options, argc, argv);
    
    if(options.input)
        compileFile(&vm, options.input, options);
    else
        repl(&vm, options);
    
    orbit_vmDeinit(&vm);
    orbit_stringPoolDeinit();
}