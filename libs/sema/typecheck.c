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

#define MATCH(type, block) case ORBIT_AST_##type: block break
#define OTHERWISE(block) default: block break

static OrbitAST* extractFuncType(Sema* self, OrbitAST* func) {
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

static OrbitAST* extractVarType(Sema* self, OrbitAST* var) {
    return var->varDecl.typeAnnotation;
}

static bool declareFunc(Sema* self, OrbitAST* func) {
    func->type = ORCRETAIN(extractFuncType(self, func));
    if(!func->type) return false;
    return declareFunction(self, func);
}

static bool declareVar(Sema* self, OrbitAST* var) {
    if(!var->type) {
        errorNoType(self, var);
        return false;
    }
    return declareVariable(self, var);
}

static bool isAssignOperator(OrbitTokenKind kind) {
    return kind == ORBIT_TOK_EQUALS
        || kind == ORBIT_TOK_PLUSEQ
        || kind == ORBIT_TOK_MINUSEQ
        || kind == ORBIT_TOK_STAREQ
        || kind == ORBIT_TOK_SLASHEQ;
}

static bool checkAssign(Sema* self, OrbitAST* assign) {
    // TODO: check that lhs is, in fact, an lvalue.
    // This isn't a given, and is probably something that will have to go in the resolver. At
    // first glance it seems like something that is composition of lvalues should be an lvalue,
    // but I have doubts (we must handle [] and () operators).
    
    OrbitAST* lhs = assign->binaryExpr.lhs;
    OrbitAST* rhs = assign->binaryExpr.rhs;
    
    // if [lhs] doesn't have a type, we are free to set it to [expr]'s type
    if(!lhs->type) {
        lhs->type = ORCRETAIN(orbit_astTypeCopy(rhs->type));
        return true;
    }
    
    // Else, we need to check that the types match (or can be converted)
    // if the types are strictly equal, roll on
    if(orbit_astTypeEquals(lhs->type, rhs->type)) return true;
    
    // else we must check that rhs -> lhs is a valid conversion, and if yes add a cast node.
    const Conversion* cast = findCast(rhs->type, lhs->type);
    if(!cast) {
        errorAssign(self, assign);
        return false;
    }
    assign->binaryExpr.rhs = ORCRETAIN(orbit_astMakeCastExpr(rhs, cast->nodeKind));
    ORCRELEASE(rhs);
    return true;
}

static void check(Sema* self, OrbitAST* node) {
    while(node) {
        switch(node->kind) {
        
            // MARK: - Declarations
            MATCH(DECL_MODULE, {
                check(self, node->moduleDecl.body);
            });
            
            MATCH(DECL_FUNC, {
                if(declareFunc(self, node)) check(self, node->funcDecl.body);
            });
            
            MATCH(DECL_VAR, {
                node->type = ORCRETAIN(extractVarType(self, node));
                declareVar(self, node);
            });
            
            // MARK: - Statements
            
        
            // MARK: - Expression Handling
            MATCH(EXPR_UNARY, {
                check(self, node->unaryExpr.rhs);
                // TODO: implement resolver
            });
        
            MATCH(EXPR_BINARY, {
                OrbitAST* lhs = node->binaryExpr.lhs;
                OrbitAST* rhs = node->binaryExpr.rhs;
                
                if(isAssignOperator(node->binaryExpr.operator.kind)) {
                    if(lhs->kind == ORBIT_AST_DECL_VAR) {
                        lhs->type = ORCRETAIN(extractVarType(self, lhs));
                        check(self, rhs);
                        if(checkAssign(self, node)) declareVariable(self, lhs);
                    } else {
                        check(self, lhs);
                        check(self, rhs);
                        checkAssign(self, node);
                    }
                } else {
                    check(self, lhs);
                    check(self, rhs);
                    resolveBinaryExpr(self, node);
                }
            });
        
            MATCH(EXPR_CALL, {
                OrbitAST* callee = node->callExpr.symbol;
                check(self, node->callExpr.params);
                if(callee->kind == ORBIT_AST_EXPR_NAME) {
                    // checkNameCall(callee, expr->callExpr.params);
                } else {
                    // checkCallable(callee, expr->callExpr.params);
                }
            });
        
            MATCH(EXPR_NAME, {
                Symbol* sym = lookupSymbol(self, node->nameExpr.name);
                if(!sym) errorNameLookup(self, node);
                else node->type = ORCRETAIN(orbit_astTypeCopy(sym->decl->type));
            });
        
            MATCH(EXPR_CONSTANT_INTEGER, {
                node->type = ORCRETAIN(orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_INT));
            });
        
            MATCH(EXPR_CONSTANT_FLOAT, {
                node->type = ORCRETAIN(orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_FLOAT));
            });
        
            MATCH(EXPR_CONSTANT_STRING, {
                node->type = ORCRETAIN(orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_STRING));
            });
        
            // MARK: - Default, do nothing
            OTHERWISE({
                warnUnimplemented(self, node);
            });
        }
        node = node->next;
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
    check(&sema, context->root);
    orbit_semaDeinit(&sema);
}

