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
#include <orbit/rt2/chunk.h>
#include <orbit/rt2/vm.h>
#include <orbit/rt2/debug.h>
#include <orbit/rt2/opcodes.h>

#include <orbit/ast/context.h>
#include <orbit/ast/diag.h>
#include <orbit/ast/traversal.h>
#include <orbit/csupport/console.h>
#include <orbit/csupport/tokens.h>
#include <orbit/csupport/string.h>
#include <orbit/parser/parser.h>
#include <orbit/sema/typecheck.h>

void emitInteger(OrbitChunk* chunk, int line, OrbitToken literal) {
    uint8_t constant = chunk->constants.count;
    orbit_arrayAppend(&chunk->constants, ORBIT_VALUE_INT(orbit_tokenDoubleValue(&literal)));
    
    orbit_chunkWrite(chunk, OP_CONST, line);
    orbit_chunkWrite(chunk, constant, line);
}

void emitDouble(OrbitChunk* chunk, int line, OrbitToken literal) {
    uint8_t constant = chunk->constants.count;
    orbit_arrayAppend(&chunk->constants, ORBIT_VALUE_FLOAT(orbit_tokenDoubleValue(&literal)));
    
    orbit_chunkWrite(chunk, OP_CONST, line);
    orbit_chunkWrite(chunk, constant, line);
}

void emitBinary(OrbitChunk* chunk, int line, OrbitTokenKind operator) {
    switch(operator) {
        case ORBIT_TOK_PLUS: orbit_chunkWrite(chunk, OP_IADD, line); break;
        case ORBIT_TOK_MINUS: orbit_chunkWrite(chunk, OP_ISUB, line); break;
        case ORBIT_TOK_STAR: orbit_chunkWrite(chunk, OP_IMUL, line); break;
        case ORBIT_TOK_SLASH: orbit_chunkWrite(chunk, OP_IDIV, line); break;
        case ORBIT_TOK_EQEQ: orbit_chunkWrite(chunk, OP_IEQ, line); break;
        case ORBIT_TOK_LT: orbit_chunkWrite(chunk, OP_ILT, line); break;
        case ORBIT_TOK_GT: orbit_chunkWrite(chunk, OP_IGT, line); break;
        case ORBIT_TOK_LTEQ: orbit_chunkWrite(chunk, OP_ILTEQ, line); break;
        case ORBIT_TOK_GTEQ: orbit_chunkWrite(chunk, OP_IGTEQ, line); break;
        default: break;
    }
}

void codegen(OrbitChunk* chunk, int line, OrbitAST* node) {
    if(!node) return;
    switch(node->kind) {
    case ORBIT_AST_EXPR_CONSTANT_INTEGER:
        emitInteger(chunk, line, node->constantExpr.symbol);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT_FLOAT:
        emitDouble(chunk, line, node->constantExpr.symbol);
        break;
        
        // case ORBIT_AST_EXPR_
        
    case ORBIT_AST_EXPR_BINARY:
        codegen(chunk, line, node->binaryExpr.lhs);
        codegen(chunk, line, node->binaryExpr.rhs);
        emitBinary(chunk, line, node->binaryExpr.operator.kind);
        break;
        
    case ORBIT_AST_DECL_MODULE:
        codegen(chunk, line, node->moduleDecl.body);
        break;
        
        default:
        break;
    }
}

OrbitResult repl_compile(OrbitChunk* chunk, int line, const char* input) {
    size_t length = strlen(input);
    char* src = malloc(length + 1);
    memcpy(src, input, length);
    src[length] = '\0';
    
    OrbitASTContext ctx;
    orbit_astContextInit(&ctx);
    ctx.source.path = "repl.orbit";
    orbit_sourceInitC(&ctx.source, src, length);
    orbit_parse(&ctx);
    sema_runTypeAnalysis(&ctx);
    orbit_diagEmitAll(&ctx.diagnostics);
    if(ctx.diagnostics.errorCount) return ORBIT_COMPILE_ERROR;
    
    //orbit_astPrint(stdout, ctx.root);
    codegen(chunk, line, ctx.root);
    // orbit_debugChunk(chunk, "repl");
    orbit_chunkWrite(chunk, OP_RETURN, line);
    orbit_astContextDeinit(&ctx);
    return ORBIT_OK;
}

void repl_forward(const char* cmd) {
    system(cmd);
}

int main(int argc, const char** argv) {
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
        
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;                             
        }
        
        if(line[0] == ':') {
            repl_forward(line+1);
            continue;
        }
        
        OrbitChunk chunk;
        orbit_chunkInit(&chunk);
        
        if(repl_compile(&chunk, lineNumber, line) == ORBIT_OK) {
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