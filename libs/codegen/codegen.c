//===--------------------------------------------------------------------------------------------===
// codegen.c - Implementation of the main code generation functions
// This source is part of Orbit
//
// Created on 2019-07-03 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/codegen/codegen.h>
#include <orbit/csupport/tokens.h>
#include <orbit/utils/memory.h>
#include <orbit/rt2/opcodes.h>
#include <orbit/rt2/invocation.h>
#include <orbit/rt2/buffer.h>
#include <orbit/rt2/value_object.h>
#include "builder.h"
#include <assert.h>

#define MATCH(type, block) case ORBIT_AST_##type: block break
#define OTHERWISE(block) default: block break

static inline OrbitValue constantInt(const OrbitAST* node) {
    assert(node->kind == ORBIT_AST_EXPR_CONSTANT_INTEGER && "cannot parse a non-integer");
    return ORBIT_VALUE_INT(orbit_tokenIntValue(&node->constantExpr.symbol));
}

static inline OrbitValue constantFloat(const OrbitAST* node) {
    assert(node->kind == ORBIT_AST_EXPR_CONSTANT_FLOAT && "cannot parse a non-float");
    return ORBIT_VALUE_FLOAT(orbit_tokenDoubleValue(&node->constantExpr.symbol));
}

static inline OrbitValue constantString(Builder* builder, const OrbitAST* node) {
    assert(node->kind == ORBIT_AST_EXPR_CONSTANT_STRING && "cannot parse a non-string");
    OCString* str = orbit_stringPoolGet(node->constantExpr.symbol.parsedStringLiteral);
    assert(str && "Invalid parsed string literal");
    return ORBIT_VALUE_REF(orbit_stringCopy(builder->gc, str->data, str->length));
}

void codegen(Builder* builder, const OrbitAST* node) {
    while(node) {
        builder->current = node;
        switch(node->kind) {
            MATCH(DECL_MODULE, {
                codegen(builder, node->moduleDecl.body);
                emitInst(builder, OP_return);
            });
            
            MATCH(DECL_VAR, {
                localVariable(builder, node->varDecl.name);
            });
            
            MATCH(BLOCK, {
                int stack = openScope(builder);
                codegen(builder, node->block.body);
                dropScope(builder, stack);
            });
            
            MATCH(CONDITIONAL, {
                codegen(builder, node->conditionalStmt.condition);
                // if the condition evaluates to false, we jump over the else-jump
                int ifJump = emitJump(builder, OP_jump_if);
                int elseJump = emitJump(builder, OP_jump);
                patchJump(builder, ifJump);
                codegen(builder, node->conditionalStmt.ifBody);
                
                if(node->conditionalStmt.elseBody) {
                    int endJump = emitJump(builder, OP_jump);
                    patchJump(builder, elseJump);
                    codegen(builder, node->conditionalStmt.elseBody);
                    patchJump(builder, endJump);
                } else {
                    patchJump(builder, elseJump);
                }
            });
            
            MATCH(WHILE, {
                
                int loopJump = offset(builder);
                codegen(builder, node->whileLoop.condition);
                int ifJump = emitJump(builder, OP_jump_if);
                int endJump = emitJump(builder, OP_jump);
                patchJump(builder, ifJump);
                codegen(builder, node->whileLoop.body);
                emitRJump(builder, OP_rjump, loopJump);
                patchJump(builder, endJump);
            });
            
            MATCH(PRINT, {
                codegen(builder, node->printStmt.expr);
                emitInst(builder, OP_print);
            });
            
            MATCH(EXPR_CONSTANT_INTEGER, {
                emitConstInst(builder, OP_const, constantInt(node));
            });
        
            MATCH(EXPR_CONSTANT_FLOAT, {
                emitConstInst(builder, OP_const, constantFloat(node));
            });
        
            MATCH(EXPR_CONSTANT_STRING, {
                emitConstInst(builder, OP_const, constantString(builder, node));
            });
            
            MATCH(EXPR_CONSTANT_BOOL, {
                if(node->constantExpr.symbol.kind == ORBIT_TOK_TRUE)
                    emitInst(builder, OP_true);
                else
                    emitInst(builder, OP_false);
            });
        
            MATCH(EXPR_UNARY, {
                // const OrbitAST* lhs = node->binaryExpr.lhs;
                // const OrbitAST* rhs = node->binaryExpr.rhs;
                // OrbitTokenKind operator = node->binaryExpr.operator.kind;
                // OrbitCode code = instSelectBinary(builder, operator, lhs, rhs);
                // emitInst(builder, code);
            });
            
            MATCH(ASSIGN, {
                const OrbitAST* lhs = node->binaryExpr.lhs;
                const OrbitAST* rhs = node->binaryExpr.rhs;
                OrbitTokenKind operator = node->binaryExpr.operator.kind;

                if(lhs->kind == ORBIT_AST_EXPR_NAME) {
                    codegen(builder, rhs);
                    emitLocalInst(builder, OP_store_local, lhs->nameExpr.name);
                } else if(lhs->kind == ORBIT_AST_DECL_VAR) {
                    codegen(builder, rhs);
                    emitLocalInst(builder, OP_store_local, lhs->varDecl.name);
                } else {
                    codegen(builder, lhs);
                    codegen(builder, rhs);
                    // TODO: there's an issue here with the whole lvalue/rvalue dealio
                }
            });
        
            MATCH(EXPR_BINARY, {
                const OrbitAST* lhs = node->binaryExpr.lhs;
                const OrbitAST* rhs = node->binaryExpr.rhs;
                OrbitTokenKind operator = node->binaryExpr.operator.kind;
                
                codegen(builder, lhs);
                codegen(builder, rhs);
                emitInst(builder, instSelect(builder, operator, lhs, rhs));
                
            });
            
            // TODO: We really should handle assignment as
            MATCH(EXPR_NAME, {
                emitLocalInst(builder, OP_load_local, node->nameExpr.name);
            });
            
            MATCH(EXPR_I2F, {
                codegen(builder, node->conversionExpr.expr);
                emitInst(builder, OP_i2f);
            });
            
            MATCH(EXPR_F2I, {
                codegen(builder, node->conversionExpr.expr);
                emitInst(builder, OP_f2i);
            });
        
            OTHERWISE({});
        }
        node = node->next;
    }
}

void orbit_codegen(OrbitGC* gc, OrbitFunction* function, OrbitASTContext* context) {
    assert(gc && "null garbage collector error");
    assert(function && "null function error");
    assert(context && "null AST context error");
    
    
    Builder builder;
    builderInit(&builder, gc);
    openFunctionGC(&builder, function);
    // builder.function = function;
    builder.context = context;
    
    codegen(&builder, context->root);
    closeFunction(&builder);
    builderDeinit(&builder);
}
