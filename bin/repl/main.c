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
#include <orbit/parser/parser.h>
#include <orbit/sema/typecheck.h>

typedef struct {
    OrbitGC* gc;
    OrbitFunction* fn;
} CodeGen;

void emitInteger(CodeGen codegen, int line, OrbitToken literal) {
    uint8_t constant = codegen.fn->constants.count;
    orbit_ValueBufferWrite(codegen.gc, &codegen.fn->constants, ORBIT_VALUE_INT(orbit_tokenIntValue(&literal)));
    
    orbit_functionWrite(codegen.gc, codegen.fn, OP_const, line);
    orbit_functionWrite(codegen.gc, codegen.fn, constant, line);
}

void emitDouble(CodeGen codegen, int line, OrbitToken literal) {
    uint8_t constant = codegen.fn->constants.count;
    orbit_ValueBufferWrite(codegen.gc, &codegen.fn->constants, ORBIT_VALUE_FLOAT(orbit_tokenDoubleValue(&literal)));
    
    orbit_functionWrite(codegen.gc, codegen.fn, OP_const, line);
    orbit_functionWrite(codegen.gc, codegen.fn, constant, line);
}

void emitString(CodeGen codegen, int line, OrbitToken literal) {
    uint8_t constant = codegen.fn->constants.count;
    
    OCString* parsed = orbit_stringPoolGet(literal.parsedStringLiteral);
    OrbitString* string = orbit_stringCopy(codegen.gc, parsed->data, parsed->length);
    
    orbit_ValueBufferWrite(codegen.gc, &codegen.fn->constants, ORBIT_VALUE_REF(string));
    
    orbit_functionWrite(codegen.gc, codegen.fn, OP_const, line);
    orbit_functionWrite(codegen.gc, codegen.fn, constant, line);
}

void emitBinary(CodeGen codegen, int line, OrbitTokenKind operator) {
    switch(operator) {
        case ORBIT_TOK_PLUS: orbit_functionWrite(codegen.gc, codegen.fn, OP_iadd, line); break;
        case ORBIT_TOK_MINUS: orbit_functionWrite(codegen.gc, codegen.fn, OP_isub, line); break;
        case ORBIT_TOK_STAR: orbit_functionWrite(codegen.gc, codegen.fn, OP_imul, line); break;
        case ORBIT_TOK_SLASH: orbit_functionWrite(codegen.gc, codegen.fn, OP_idiv, line); break;
        case ORBIT_TOK_EQEQ: orbit_functionWrite(codegen.gc, codegen.fn, OP_ieq, line); break;
        case ORBIT_TOK_LT: orbit_functionWrite(codegen.gc, codegen.fn, OP_ilt, line); break;
        case ORBIT_TOK_GT: orbit_functionWrite(codegen.gc, codegen.fn, OP_igt, line); break;
        case ORBIT_TOK_LTEQ: orbit_functionWrite(codegen.gc, codegen.fn, OP_ilteq, line); break;
        case ORBIT_TOK_GTEQ: orbit_functionWrite(codegen.gc, codegen.fn, OP_igteq, line); break;
        default: break;
    }
}

void emit(CodeGen codegen, int line, OrbitAST* node) {
    if(!node) return;
    switch(node->kind) {
    case ORBIT_AST_EXPR_CONSTANT_INTEGER:
        emitInteger(codegen, line, node->constantExpr.symbol);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT_FLOAT:
        emitDouble(codegen, line, node->constantExpr.symbol);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT_STRING:
        emitString(codegen, line, node->constantExpr.symbol);
        break;
        
        // case ORBIT_AST_EXPR_
        
    case ORBIT_AST_EXPR_BINARY:
        emit(codegen, line, node->binaryExpr.lhs);
        emit(codegen, line, node->binaryExpr.rhs);
        emitBinary(codegen, line, node->binaryExpr.operator.kind);
        break;
        
    case ORBIT_AST_DECL_MODULE:
        emit(codegen, line, node->moduleDecl.body);
        break;
        
        default:
        break;
    }
}

OrbitResult repl_compile(CodeGen codegen, int line, const char* input) {
    size_t length = strlen(input);
    char* src = malloc(length + 1);
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
    
    emit(codegen, line, ctx.root);
    orbit_debugFunction(codegen.fn, "repl");
    orbit_functionWrite(codegen.gc, codegen.fn, OP_return, 1);
    
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
        CodeGen codegen = (CodeGen){&vm->gc, fn};
        
        if(repl_compile(codegen, lineNumber, line) == ORBIT_OK) {
            orbit_run(vm, fn);
            console_setColor(stderr, CLI_CYAN);
            orbit_debugTOS(vm);
            console_setColor(stderr, CLI_RESET);
        }
        
        orbit_gcPop(&vm->gc);
        orbit_gcRun(&vm->gc);
        lineNumber += 1;
    }
}

OrbitResult compileFile(OrbitVM* vm, const char* path) {
    OrbitFunction* fn = orbit_functionNew(&vm->gc);
    orbit_gcPush(&vm->gc, (OrbitObject*)fn);
    CodeGen codegen = (CodeGen){&vm->gc, fn};
    
    OrbitASTContext ctx;
    orbit_astContextInit(&ctx);
    if(!orbit_sourceInitPath(&ctx.source, path))
        fprintf(stderr, "error: cannot open source file '%s'\n", path);
    orbit_parse(&ctx);
    
    orbit_semaCheck(&ctx);
    orbit_diagEmitAll(&ctx.diagnostics);
    orbit_astPrint(stdout, ctx.root);
    if(ctx.diagnostics.errorCount) return ORBIT_COMPILE_ERROR;
    
    emit(codegen, 1, ctx.root);
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