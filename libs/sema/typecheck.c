//===--------------------------------------------------------------------------------------------===
// typecheck.c - Implementation of the main type checking functions of Orbit
// This source is part of Orbit
//
// Created on 2019-07-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/sema/typecheck.h>
#include <orbit/utils/memory.h>
#include <orbit/ast/builders.h>
#include <orbit/ast/type.h>
#include <assert.h>

#include "sema_private.h"
#include "expr_resolver.h"
#include "helpers.h"
#include "errors.h"

static OrbitAST* funcType(Sema* self, OrbitAST* func) {
    OrbitAST* returnType = func->funcDecl.returnType ? func->funcDecl.returnType : orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_VOID);
    
    ASTListBuilder params;
    orbit_astListStart(&params);
    OrbitAST* param = func->funcDecl.params;
    while(param) {
        orbit_astListAdd(&params, orbit_astTypeCopy(param->type));
        param = param->next;
    }
    return orbit_astMakeFuncType(returnType, orbit_astListClose(&params));
}

static OrbitAST* varType(Sema* self, OrbitAST* var, bool mustAnnotate) {
    // If there's a type annotation, we hoist that into the AST node type. If not,
    // we punt until the assignment binary expression is typed, at which point it'll assign
    // the resulting deduced type.
    if(!var->varDecl.typeAnnotation) {
    printf("varType(, , %s)\n", mustAnnotate ? "true" : "false");
        if(mustAnnotate) {
            errorNoType(self, var);
        }
        return NULL;
    }
    
    // TODO: canonical-ise the type if possible, which should speed up comparisons/etc.
    return orbit_astTypeCopy(var->varDecl.typeAnnotation);
}

static OrbitAST* exprType(Sema* self, OrbitAST* expr) {
    
#define MATCH(type, block) case ORBIT_AST_##type: block break
#define OTHERWISE(block) default: block break
    
    switch(expr->kind) {
        MATCH(EXPR_UNARY, {
            OrbitAST* rhs = exprType(self, expr->unaryExpr.rhs);
            // TODO: this is just wrong. Like actually wrong.
            expr->type = ORCRETAIN(orbit_astTypeCopy(rhs));
            // expr->type = resolve(self, node->unaryExpr.operator, rhs);
        });
        
        MATCH(EXPR_BINARY, {
            OrbitAST* lhs = expr->binaryExpr.lhs;
            if(lhs->kind == ORBIT_AST_DECL_VAR) {
                lhs->type = ORCRETAIN(varType(self, lhs, false));
            } else {
                exprType(self, lhs);
            }
            
            exprType(self, expr->binaryExpr.rhs);
            
            resolveBinaryExpr(self, expr);
        });
        
        MATCH(EXPR_CALL, {
            OrbitAST* callee = expr->callExpr.symbol;
            if(callee->kind == ORBIT_AST_EXPR_NAME) {
                
            } else {
                
            }
        });
        
        MATCH(EXPR_NAME, {
            Symbol* decl = lookupSymbol(self, expr->nameExpr.name);
            if(decl) {
                
            } else {
                
            }
        });
        
        MATCH(EXPR_CONSTANT_INTEGER, {
            expr->type = ORCRETAIN(orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_INT));
        });
        
        MATCH(EXPR_CONSTANT_FLOAT, {
            expr->type = ORCRETAIN(orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_FLOAT));
        });
        
        MATCH(EXPR_CONSTANT_STRING, {
            expr->type = ORCRETAIN(orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_STRING));
        });
        
        OTHERWISE({});
     }
    
     return expr->type;
}

static void checkStmt(Sema* self, OrbitAST* node) {
    if(node->kind & ASTExprMask) {
        exprType(self, node);
        return;
    }
    
    switch(node->kind) {
        MATCH(DECL_VAR, {
            node->type = ORCRETAIN(varType(self, node, true));
        });
        
        OTHERWISE({});
    }
    
}


static void checkNode(Sema* self, OrbitAST* node) {
    switch(node->kind) {

        MATCH(BLOCK, {
            pushScope(self);
            checkStmt(self, node->block.body);
            popScope(self);
        });
        
        MATCH(DECL_MODULE, {
            pushScope(self);
            checkStmt(self, node->moduleDecl.body);
            popScope(self);
        });
        
        MATCH(DECL_FUNC, {
            node->type = ORCRETAIN(funcType(self, node));
        });
        
        MATCH(DECL_VAR, {
            node->type = ORCRETAIN(varType(self, node, true));
        });
        
        OTHERWISE({});
    }
}


void orbit_semaCheck(OrbitASTContext* context) {
    assert(context && "null syntax tree error");
    
    Sema sema;
    orbit_semaInit(&sema);
    sema.context = context;
    declareDefaultOperators(&sema);
    
    // We need to walk the tree and
    //  1) declare symbols as we encounter them
    //  2) infer type of symbols
    //  3) deduce type of expressions and check them
    //      3a) insert conversion nodes where needed (i2f, f2i)
    //  4) Check function calls?
    //
    //
    // As enticing as it is, not quite sure the AST walker/traversal API is the best tool for
    // that job -- might be easier to roll our own here instead.
    //
    // We also need to match function overloads and implicit type conversions (not many really,
    // only Int <-> Float).
    // Overloads can be handled through a linked list of Symbol structs, which get stored in the
    // symbol table. 
    // The conversion system probably belongs in a sibling module with sort-of instruction
    // selecting. Given that not all operations will be handled through native instructions
    // (yay operator overloading), it's probably best to put all of that behind some layer of
    // abstraction so we don't have to come in here and pull everything apart when we implement
    // that.
    checkNode(&sema, context->root);
    orbit_semaDeinit(&sema);
}

