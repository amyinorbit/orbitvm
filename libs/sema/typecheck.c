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
#include <orbit/mangling/mangle.h>
#include <assert.h>

#include "sema_private.h"
#include "expr_resolver.h"
#include "helpers.h"
#include "errors.h"

#define MATCH(type, block) case ORBIT_AST_##type: block break
#define OTHERWISE(block) default: block break

static OrbitAST* extractFuncType(Sema* self, OrbitAST* func) {
    OrbitAST* returnType = func->funcDecl.returnType ?
        func->funcDecl.returnType : orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_VOID);
    
    ASTListBuilder params;
    orbit_astListStart(&params);
    OrbitAST* param = func->funcDecl.params;
    while(param) {
        printf("found 1 param\n");
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
    func->funcDecl.mangledName = orbit_mangleFuncName(func);
    return declareFunction(self, func);
}

static bool declareFuncParams(Sema* self, OrbitAST* decl) {
    OrbitAST* param = decl->funcDecl.params;
    while(param) {
        if(!declareVariable(self, param)) return false;
        param = param->next;
    }
    return true;
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

typedef enum {MATCH_NONE, MATCH_CAST, MATCH_STRICT} ParamMatch;

static ParamMatch checkArgTypes(const OrbitAST* paramTypes, const OrbitAST* args) {
    const OrbitAST* paramType = paramTypes;
    
    ParamMatch match = MATCH_STRICT;
    
    for(const OrbitAST* arg = args; arg != NULL; arg = arg->next, paramType = paramType->next) {
        printf("[arg->%p]\n", arg->next);
        if(!paramType) {puts("noParam\n");return MATCH_NONE;}
        if(orbit_astTypeEquals(arg->type, paramType)) continue;
        match = MATCH_CAST;
        if(!findCast(arg->type, paramType)) {puts("noCast\n"); return MATCH_NONE; }
    }
    // we must be at the end of the parameter type list too.
    return paramType == NULL ? match : MATCH_NONE; 
}

static bool finishCallExpr(Sema* self, OrbitAST* decl, OrbitAST* call, bool strict) {
    
    OrbitAST* type = decl->type;
    OrbitAST* callee = call->callExpr.symbol;
    
    callee->type = ORCRETAIN(orbit_astTypeCopy(decl->type));
    call->type= ORCRETAIN(orbit_astTypeCopy(type->typeExpr.funcType.returnType));
    if(strict) return true;
    
    const OrbitAST* paramType = type->typeExpr.funcType.params;
    // const OrbitAST* args = call->callExpr.params;
    OrbitAST** argPtr = &call->callExpr.params;
    for(; *argPtr != NULL; argPtr = &(*argPtr)->next, paramType = paramType->next) {
        assert(paramType && "invalid function binding");
        
        if(orbit_astTypeEquals((*argPtr)->type, paramType)) continue;
        const Conversion* cast = findCast((*argPtr)->type, paramType);
        assert(cast && "invalid function binding");
        
        OrbitAST* wrapped = ORCRETAIN(orbit_astMakeCastExpr(*argPtr, cast->nodeKind));
        ORCRELEASE(*argPtr);
        wrapped->next = (*argPtr)->next;
        wrapped->type = ORCRETAIN(orbit_astTypeCopy(paramType));
        (*argPtr)->next = NULL;
        *argPtr = wrapped;
    }
    assert(!paramType && "invalid function binding");
    
    return true;
}

static bool checkCallable(Sema* self, OrbitAST* call) {
    OrbitAST* callee = call->callExpr.symbol;
    OrbitAST* type = callee->type;
    OrbitAST* args = call->callExpr.params;
    if(callee->type->kind != ORBIT_AST_TYPEEXPR_FUNC) {
        errorNotCallable(self, call);
        return false;
    }
    
    ParamMatch match = checkArgTypes(type->typeExpr.funcType.params, args);
    if(match == MATCH_STRICT)
        return finishCallExpr(self, callee, call, true);
    if(match == MATCH_CAST)
        return finishCallExpr(self, callee, call, false);
    
    errorInvalidCall(self, call);
    return false;
}

static bool checkNameCall(Sema* self, OrbitAST* call) {
    OrbitAST* callee = call->callExpr.symbol;
    OCStringID name = callee->nameExpr.name;
    
    OrbitAST* cast = NULL;
    for(Symbol* sym = lookupSymbol(self, name); sym != NULL; sym = sym->next) {
        OrbitAST* type = sym->decl->type;
        if(type->kind != ORBIT_AST_TYPEEXPR_FUNC)
            continue;
        ParamMatch match = checkArgTypes(type->typeExpr.funcType.params, call->callExpr.params);
        if(match == MATCH_NONE)
            continue;
        if(match == MATCH_STRICT)
            return finishCallExpr(self, sym->decl, call, true);
        cast = sym->decl;
    }
    
    if(cast) return finishCallExpr(self, cast, call, false);
    
    errorInvalidCall(self, call);
    return false;
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
            
            MATCH(CONDITIONAL, {
                OrbitAST* expr = node->conditionalStmt.condition;
                check(self, expr);
                if(!orbit_astTypeEqualsPrimitive(expr->type, ORBIT_AST_TYPEEXPR_BOOL)) {
                    errorCondition(self, "if statement", expr);
                }
                check(self, node->conditionalStmt.ifBody);
                check(self, node->conditionalStmt.elseBody);
            });
            
            MATCH(WHILE, {
                OrbitAST* expr = node->whileLoop.condition;
                check(self, expr);
                if(!orbit_astTypeEqualsPrimitive(expr->type, ORBIT_AST_TYPEEXPR_BOOL)) {
                    errorCondition(self, "while loop", expr);
                }
                check(self, node->whileLoop.body);
            });
            
            // TODO: we should be checking that the expression's type does match the function being
            // checked.
            MATCH(RETURN, {
                OrbitAST* expr = node->returnStmt.returnValue;
                check(self, expr);
                if(expr) {
                    node->type = ORCRETAIN(orbit_astTypeCopy(expr->type));
                }
            });
            
            MATCH(PRINT, {
                check(self, node->printStmt.expr);
            });
            
            MATCH(BLOCK, {
                pushScope(self);
                check(self, node->block.body);
                popScope(self);
            });
            
            // MARK: - Declarations
            MATCH(DECL_MODULE, {
                check(self, node->moduleDecl.body);
            });
            
            MATCH(DECL_FUNC, {
                //fun test = (a: Int) -> Int { return a }; test(123)
                check(self, node->funcDecl.params);
                if(declareFunc(self, node)) {
                    pushScope(self);
                    declareFuncParams(self, node);
                    check(self, node->funcDecl.body);
                    popScope(self);
                }
                    
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
                    checkNameCall(self, node);
                } else {
                    check(self, callee);
                    checkCallable(self, node);
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
            
            MATCH(EXPR_CONSTANT_BOOL, {
                node->type = ORCRETAIN(orbit_astMakePrimitiveType(ORBIT_AST_TYPEEXPR_BOOL));
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
    check(&sema, context->root);
    orbit_semaDeinit(&sema);
}

