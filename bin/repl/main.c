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
#include <term/line.h>

typedef struct {
    OrbitGC* gc;
    OrbitFunction* fn;
} Compiler;

typedef struct {
    bool dumpAST;
    bool dumpJSON;
    bool dumpBytecode;
    bool codegen;
    bool run;
    const char* input;
} Options;

OrbitResult repl_compile(Compiler* comp, int line, char* input, Options options) {
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
    if(options.dumpAST && !options.dumpJSON) orbitASTPrint(stdout, ctx.root);
    if(options.dumpJSON && !options.dumpAST) orbitASTJSON(stdout, ctx.root);
    if(ctx.diagnostics.errorCount) return ORBIT_COMPILE_ERROR;

    comp->fn = orbitCodegen(comp->gc, &ctx);
    orbitFunctionWrite(comp->gc, comp->fn, OP_return_repl, 1);
    if(options.dumpBytecode) orbitDebugFunction(comp->fn, "repl");

    orbitASTContextDeinit(&ctx);
    return ORBIT_OK;
}

void prompt(const char* PS) {
    termColorFG(stdout, kTermBlue);
    printf("orbit:%s> ", PS);
    termColorFG(stdout, kTermDefault);
}

static const char* historyPath() {
    const char* home = getenv("HOME");
    if(!home) return ".orbit_history";

    static char path[4096];
    snprintf(path, 4096, "%s/.orbit_history", home);
    return path;
}

void repl(OrbitVM* vm, Options options) {

    printf("Welcome to Orbit version 2020.2 repl (" __DATE__ ")\n");
    printf("[built with " __COMPILER_NAME__ "]\n");
    int lineNumber = 1;
    char promptBuffer[6];
    char* source = NULL;

    LineFunctions fn = {.printPrompt = &prompt};
    Line* editor = lineNew(&fn);
    lineHistoryLoad(editor, historyPath());

    snprintf(promptBuffer, 6, "%3d", lineNumber);
    lineSetPrompt(editor, promptBuffer);

    while((source = lineGet(editor))) {
        // OrbitFunction* fn = orbitFunctionNew(&vm->gc);
        // orbitGCPush(&vm->gc, (OrbitObject*)fn);
        Compiler comp = (Compiler){&vm->gc, NULL};

        if(repl_compile(&comp, lineNumber, source, options) == ORBIT_OK) {
            orbitRun(vm, comp.fn);
            termColorFG(stderr, kTermGreen);
            orbitDebugTOS(vm);
            // orbitDebugStack(vm);
            termColorFG(stderr, kTermDefault);
            orbitGCRun(&vm->gc);

            lineNumber += 1;
            snprintf(promptBuffer, 6, "%3d", lineNumber);
        }

        orbitGCPop(&vm->gc);
        orbitGCRun(&vm->gc);
    }

    lineHistoryWrite(editor, historyPath());
    lineDealloc(editor);
}

OrbitResult compileFile(OrbitVM* vm, const char* path, Options options) {
    // OrbitFunction* fn = orbitFunctionNew(&vm->gc);
    // orbitGCPush(&vm->gc, (OrbitObject*)fn);
    Compiler comp = (Compiler){&vm->gc, NULL};

    OrbitASTContext ctx;
    orbitASTContextInit(&ctx);
    if(!orbitSourceInitPath(&ctx.source, path))
        termError("orbit", -1, "cannot open source file '%s'", path);
    orbitParse(&ctx);

    orbitSemaCheck(&ctx);
    orbitDiagEmitAll(&ctx.diagnostics);
    if(options.dumpAST && !options.dumpJSON) orbitASTPrint(stdout, ctx.root);
    if(options.dumpJSON && !options.dumpAST) orbitASTJSON(stdout, ctx.root);
    if(ctx.diagnostics.errorCount) return ORBIT_COMPILE_ERROR;

    if(!options.codegen) goto cleanup;

    comp.fn = orbitCodegen(comp.gc, &ctx);
    if(options.dumpBytecode) orbitDebugFunction(comp.fn, path);
    orbitFunctionWrite(&vm->gc, comp.fn, OP_return, 1);

    if(!options.run) goto cleanup;

    orbitRun(vm, comp.fn);

cleanup:
    orbitASTContextDeinit(&ctx);
    orbitGCRun(&vm->gc);
    return ORBIT_OK;
}

#include "cli_options.inc"

static void printVersion() {
    printf("Orbit version 2020.2 (" __DATE__ ")\n");
    printf("built with " __COMPILER_NAME__ "\n");
    printf("Copyright (c) 2016-2020 Amy Parent <amy@amyparent.com>\n");
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
        case 'j':
            options->dumpJSON = true;
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
    Options options = (Options){false, false, false, true, true, NULL};
    parseArguments(&options, argc, argv);

    if(options.input)
        compileFile(&vm, options.input, options);
    else
        repl(&vm, options);

    orbitVMDeinit(&vm);
    orbitStringPoolDeinit();
}
