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
    OrbitChunk* chunk;
} CodeGen;

void emitInteger(CodeGen codegen, int line, OrbitToken literal) {
    uint8_t constant = codegen.chunk->constants.count;
    orbit_arrayAppend(&codegen.chunk->constants, ORBIT_VALUE_INT(orbit_tokenIntValue(&literal)));
    
    orbit_chunkWrite(codegen.chunk, OP_CONST, line);
    orbit_chunkWrite(codegen.chunk, constant, line);
}

void emitDouble(CodeGen codegen, int line, OrbitToken literal) {
    uint8_t constant = codegen.chunk->constants.count;
    orbit_arrayAppend(&codegen.chunk->constants, ORBIT_VALUE_FLOAT(orbit_tokenDoubleValue(&literal)));
    
    orbit_chunkWrite(codegen.chunk, OP_CONST, line);
    orbit_chunkWrite(codegen.chunk, constant, line);
}

void emitString(CodeGen codegen, int line, OrbitToken literal) {
    uint8_t constant = codegen.chunk->constants.count;
    
    OCString* parsed = orbit_stringPoolGet(literal.parsedStringLiteral);
    OrbitString* string = orbit_stringCopy(codegen.gc, parsed->data, parsed->length);
    
    orbit_arrayAppend(&codegen.chunk->constants, ORBIT_VALUE_REF(string));
    
    orbit_chunkWrite(codegen.chunk, OP_CONST, line);
    orbit_chunkWrite(codegen.chunk, constant, line);
}

void emitBinary(CodeGen codegen, int line, OrbitTokenKind operator) {
    switch(operator) {
        case ORBIT_TOK_PLUS: orbit_chunkWrite(codegen.chunk, OP_IADD, line); break;
        case ORBIT_TOK_MINUS: orbit_chunkWrite(codegen.chunk, OP_ISUB, line); break;
        case ORBIT_TOK_STAR: orbit_chunkWrite(codegen.chunk, OP_IMUL, line); break;
        case ORBIT_TOK_SLASH: orbit_chunkWrite(codegen.chunk, OP_IDIV, line); break;
        case ORBIT_TOK_EQEQ: orbit_chunkWrite(codegen.chunk, OP_IEQ, line); break;
        case ORBIT_TOK_LT: orbit_chunkWrite(codegen.chunk, OP_ILT, line); break;
        case ORBIT_TOK_GT: orbit_chunkWrite(codegen.chunk, OP_IGT, line); break;
        case ORBIT_TOK_LTEQ: orbit_chunkWrite(codegen.chunk, OP_ILTEQ, line); break;
        case ORBIT_TOK_GTEQ: orbit_chunkWrite(codegen.chunk, OP_IGTEQ, line); break;
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
    //sema_runTypeAnalysis(&ctx);
    orbit_diagEmitAll(&ctx.diagnostics);
    if(ctx.diagnostics.errorCount) return ORBIT_COMPILE_ERROR;
    
    orbit_astPrint(stdout, ctx.root);
    emit(codegen, line, ctx.root);
    // orbit_debugChunk(chunk, "repl");
    orbit_chunkWrite(codegen.chunk, OP_RETURN, line);
    orbit_astContextDeinit(&ctx);
    return ORBIT_OK;
}

void repl_forward(const char* cmd) {
    system(cmd);
}

int main(int argc, const char** argv) {
    setlocale(LC_ALL, NULL);
    orbit_stringPoolInit(1024);
    
    OrbitVM vm;
    orbit_vmInit(&vm);
    
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
        
        if(line[0] == ':') {
            repl_forward(line+1);
            continue;
        }
        
        OrbitChunk chunk;
        orbit_chunkInit(&chunk);
        CodeGen codegen = (CodeGen){&vm.gc, &chunk};
        
        if(repl_compile(codegen, lineNumber, line) == ORBIT_OK) {
            orbit_run(&vm, &chunk);
            console_setColor(stderr, CLI_CYAN);
            orbit_debugTOS(&vm);
            console_setColor(stderr, CLI_RESET);
        }
        
        orbit_chunkDeinit(&chunk);
        lineNumber += 1;
    }
    orbit_vmDeinit(&vm);
    
    
}