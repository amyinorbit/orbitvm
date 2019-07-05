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
        
            MATCH(EXPR_UNARY, {
                // const OrbitAST* lhs = node->binaryExpr.lhs;
                // const OrbitAST* rhs = node->binaryExpr.rhs;
                // OrbitTokenKind operator = node->binaryExpr.operator.kind;
                // OrbitCode code = instSelectBinary(builder, operator, lhs, rhs);
                // emitInst(builder, code);
            });
        
            MATCH(EXPR_BINARY, {
                const OrbitAST* lhs = node->binaryExpr.lhs;
                const OrbitAST* rhs = node->binaryExpr.rhs;
                codegen(builder, lhs);
                codegen(builder, rhs);
                OrbitTokenKind operator = node->binaryExpr.operator.kind;
                emitInst(builder, instSelect(builder, operator, lhs, rhs));
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
    builder.function = function;
    builder.context = context;
    
    codegen(&builder, context->root);
    builderDeinit(&builder);
}
