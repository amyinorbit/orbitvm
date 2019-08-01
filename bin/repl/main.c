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
#include <term/colors.h>
#include <term/printing.h>
#include <term/repl.h>

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

OrbitResult repl_compile(Compiler comp, int line, char* input, Options options) {
    size_t length = strlen(input);
    // char* src = orbitAllocator(NULL, 0, length + 1);
    // memcpy(src, input, length);
    // src[length] = '\0';

    OrbitASTContext ctx;
    orbitASTContextInit(&ctx);
    ctx.source.path = "repl.orbit";
    orbitSourceInitC(&ctx.source, input, length);
    orbitParse(&ctx);

    orbitSemaCheck(&ctx);
    orbitDiagEmitAll(&ctx.diagnostics);
    if(options.dumpAST) orbitASTPrint(stdout, ctx.root);
    if(ctx.diagnostics.errorCount) return ORBIT_COMPILE_ERROR;

    orbitCodegen(comp.gc, comp.fn, &ctx);
    orbitFunctionWrite(comp.gc, comp.fn, OP_return_repl, 1);
    if(options.dumpBytecode) orbitDebugFunction(comp.fn, "repl");

    orbitASTContextDeinit(&ctx);
    return ORBIT_OK;
}


void repl(OrbitVM* vm, Options options) {

    printf("Welcome to Orbit version 2019.6 repl (" __DATE__ ")\n");
    printf("[built with " __COMPILER_NAME__ "]\n");
    int lineNumber = 1;


    TermREPL repl;
    termREPLInit(&repl);
    char* source = NULL;

    while((source = termREPL(&repl, "orbit "))) {
        OrbitFunction* fn = orbitFunctionNew(&vm->gc);
        orbitGCPush(&vm->gc, (OrbitObject*)fn);
        Compiler comp = (Compiler){&vm->gc, fn};

        if(repl_compile(comp, lineNumber, source, options) == ORBIT_OK) {
            orbitRun(vm, fn);
            termColorFG(stderr, kTermGreen);
            orbitDebugTOS(vm);
            // orbitDebugStack(vm);
            termColorFG(stderr, kTermDefault);
            orbitGCRun(&vm->gc);
            lineNumber += 1;
        }

        orbitGCPop(&vm->gc);
        orbitGCRun(&vm->gc);
    }

    termREPLDeinit(&repl);
}

OrbitResult compileFile(OrbitVM* vm, const char* path, Options options) {
    OrbitFunction* fn = orbitFunctionNew(&vm->gc);
    orbitGCPush(&vm->gc, (OrbitObject*)fn);
    Compiler comp = (Compiler){&vm->gc, fn};

    OrbitASTContext ctx;
    orbitASTContextInit(&ctx);
    if(!orbitSourceInitPath(&ctx.source, path))
        termError("orbit", -1, "cannot open source file '%s'", path);
    orbitParse(&ctx);

    orbitSemaCheck(&ctx);
    orbitDiagEmitAll(&ctx.diagnostics);
    if(options.dumpAST) orbitASTPrint(stdout, ctx.root);
    if(ctx.diagnostics.errorCount) return ORBIT_COMPILE_ERROR;

    if(!options.codegen) goto cleanup;

    orbitCodegen(comp.gc, comp.fn, &ctx);
    if(options.dumpBytecode) orbitDebugFunction(fn, path);
    orbitFunctionWrite(&vm->gc, fn, OP_return, 1);

    if(!options.run) goto cleanup;

    orbitRun(vm, fn);

cleanup:
    orbitASTContextDeinit(&ctx);
    orbitGCRun(&vm->gc);
    return ORBIT_OK;
}

#include "cli_options.inc"

static void printVersion() {
    printf("Orbit version 2019.6 (" __DATE__ ")\n");
    printf("built with " __COMPILER_NAME__ "\n");
    printf("Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>\n");
}

static _Noreturn void printHelp() {
    printVersion();
    puts("");
    termPrintUsage(stdout, "orbit", kOrbitUses, kOrbitUseCount);
    puts("");
    termPrintHelp(stdout, kOrbitParams, kOrbitParamCount);
    puts("");
    termPrintBugreports(stdout, "orbit", kOrbitEmail, kOrbitWebsite);
    exit(0);
}

static void parseArguments(Options* options, int argc, const char** argv) {

    TermArgParser parser;
    termArgParserInit(&parser, argc, argv);

    TermArgResult r = termArgParse(&parser, kOrbitParams, kOrbitParamCount);
    while(r.name != kTermArgDone) {
        switch(r.name) {
        case kTermArgHelp:
            printHelp();
            break;

        case kTermArgVersion:
            printVersion();
            exit(0);
            break;

        case kTermArgError:
            termError("orbit", 1, "%s", parser.error);
            break;

        case 'g':
            options->dumpAST = true;
            options->dumpBytecode = true;
            break;
        case 'n':
            options->codegen = false;
            break;
        case 't':
        case 'T':
            options->dumpAST = true;
            break;
        case 'x':
        case 'S':
            options->dumpBytecode = true;
            break;
        case kTermArgPositional:
            options->input = r.value;
            break;
        }

        r = termArgParse(&parser, kOrbitParams, kOrbitParamCount);
    }
}

int main(int argc, const char** argv) {
    setlocale(LC_ALL, NULL);
    orbitStringPoolInit(1024);

    OrbitVM vm;
    orbitVMInit(&vm);

    // const char* inputFile = NULL;
    Options options = (Options){false, false, true, true, NULL};
    parseArguments(&options, argc, argv);

    if(options.input)
        compileFile(&vm, options.input, options);
    else
        repl(&vm, options);

    orbitVMDeinit(&vm);
    orbitStringPoolDeinit();
}
