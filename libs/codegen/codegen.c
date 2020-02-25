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
#include <string.h>


#define GC(fn) ((fn)->context->gc)
#define MODULE(fn) ((fn)->context->module->functions)
#define GC_FUNC() fn->impl
#define SEL(fn) ((fn)->context->selector)
#define CTX(fn) ((fn)->context)

#define OTHERWISE(block) default: block break

static inline OrbitValue constantInt(const OrbitAST* node) {
    assert(node->kind == ORBIT_AST_EXPR_CONSTANT_INTEGER && "cannot parse a non-integer");
    return ORBIT_VALUE_INT(orbitTokenIntValue(&node->constantExpr.symbol));
}

static inline OrbitValue constantFloat(const OrbitAST* node) {
    assert(node->kind == ORBIT_AST_EXPR_CONSTANT_FLOAT && "cannot parse a non-float");
    return ORBIT_VALUE_FLOAT(orbitTokenDoubleValue(&node->constantExpr.symbol));
}

static inline OrbitValue constantString(Function* builder, const OrbitAST* node) {
    assert(node->kind == ORBIT_AST_EXPR_CONSTANT_STRING && "cannot parse a non-string");
    OCString* str = orbitStringPoolGet(node->constantExpr.symbol.parsedStringLiteral);
    assert(str && "Invalid parsed string literal");
    return ORBIT_VALUE_REF(orbitStringCopy(GC(builder), str->data, str->length));
}

void codegen(Function* builder, const OrbitAST* node) {
    while(node) {
        // builder->current = node;
        switch(node->kind) {
            case ORBIT_AST_DECL_MODULE: {
                codegen(builder, node->moduleDecl.body);
                // emitInst(builder, OP_return);
            } break;

            case ORBIT_AST_DECL_VAR: {
                localVariable(builder, node->varDecl.name);
            } break;

            case ORBIT_AST_DECL_FUNC: {

                Function fn;
                openFunction(CTX(builder), &fn, node->funcDecl.mangledName);
                int stack = openScope(&fn);
                codegen(&fn, node->funcDecl.params);
                codegen(&fn, node->funcDecl.body);

                dropScope(&fn, stack);
                closeFunction(&fn);
            } break;

            case ORBIT_AST_BLOCK: {
                int stack = openScope(builder);
                codegen(builder, node->block.body);
                dropScope(builder, stack);
            } break;

            case ORBIT_AST_CONDITIONAL: {
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
            } break;

            case ORBIT_AST_WHILE: {

                int loopJump = offset(builder);
                codegen(builder, node->whileLoop.condition);
                int ifJump = emitJump(builder, OP_jump_if);
                int endJump = emitJump(builder, OP_jump);
                patchJump(builder, ifJump);
                codegen(builder, node->whileLoop.body);
                emitRJump(builder, OP_rjump, loopJump);
                patchJump(builder, endJump);
            } break;

            case ORBIT_AST_PRINT: {
                codegen(builder, node->printStmt.expr);
                emitInst(builder, OP_print);
            } break;

            case ORBIT_AST_EXPR_CONSTANT_INTEGER: {
                emitConstInst(builder, OP_const, constantInt(node));
            } break;

            case ORBIT_AST_EXPR_CONSTANT_FLOAT: {
                emitConstInst(builder, OP_const, constantFloat(node));
            } break;

            case ORBIT_AST_EXPR_CONSTANT_STRING: {
                emitConstInst(builder, OP_const, constantString(builder, node));
            } break;

            case ORBIT_AST_EXPR_CONSTANT_BOOL: {
                if(node->constantExpr.symbol.kind == ORBIT_TOK_TRUE)
                    emitInst(builder, OP_true);
                else
                    emitInst(builder, OP_false);
            } break;

            case ORBIT_AST_EXPR_UNARY: {
                // const OrbitAST* lhs = node->binaryExpr.lhs;
                // const OrbitAST* rhs = node->binaryExpr.rhs;
                // OrbitTokenKind operator = node->binaryExpr.operator.kind;
                // OrbitCode code = instSelectBinary(builder, operator, lhs, rhs);
                // emitInst(builder, code);
            } break;

            case ORBIT_AST_ASSIGN: {
                const OrbitAST* lhs = node->binaryExpr.lhs;
                const OrbitAST* rhs = node->binaryExpr.rhs;
                // OrbitTokenKind operator = node->binaryExpr.operator.kind;

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
            } break;

            case ORBIT_AST_EXPR_BINARY: {
                const OrbitAST* lhs = node->binaryExpr.lhs;
                const OrbitAST* rhs = node->binaryExpr.rhs;
                OrbitTokenKind operator = node->binaryExpr.operator.kind;

                codegen(builder, lhs);
                codegen(builder, rhs);
                emitInst(builder, instSelect(builder, operator, lhs, rhs));

            } break;

            // TODO: We really should handle assignment as
            case ORBIT_AST_EXPR_NAME: {
                emitLocalInst(builder, OP_load_local, node->nameExpr.name);
            } break;

            case ORBIT_AST_EXPR_I2F: {
                codegen(builder, node->conversionExpr.expr);
                emitInst(builder, OP_i2f);
            } break;

            case ORBIT_AST_EXPR_F2I: {
                codegen(builder, node->conversionExpr.expr);
                emitInst(builder, OP_f2i);
            } break;

            case ORBIT_AST_EXPR_CALL: {
                codegen(builder, node->callExpr.params);
                if(node->callExpr.callee != orbitInvalidStringID) {
                    emitLocalInst(builder, OP_load_local, node->callExpr.callee);
                } else {
                    codegen(builder, node->callExpr.symbol);
                }
                emitInst(builder, OP_call);
            } break;

            case ORBIT_AST_RETURN: {
                if(node->returnStmt.returnValue) {
                    codegen(builder, node->returnStmt.returnValue);
                    emitInst(builder, OP_return_val);
                } else {
                    emitInst(builder, OP_return);
                }
            } break;

            default: break;
        }
        node = node->next;
    }
}

// TODO: this need refactored. Clox's method of creating one compiler per function
// comes to mind and might be a good idea to simplify how we're doing this.
//
// Another thing that could be worth it is simplifying things a lot and getting rid of the AST
OrbitFunction* orbitCodegen(OrbitGC* gc, OrbitASTContext* context) {
    assert(gc && "null garbage collector error");
    assert(context && "null AST context error");

    Codegen gen;
    contextInit(&gen, gc);
    gen.ast = context;

    Function root;
    openFunction(&gen, &root, orbitStringIntern("<script>", strlen("<script>")));
    codegen(&root, context->root);
    OrbitFunction* fn = closeFunction(&root);
    contextDeinit(&gen);

    return fn;
}
