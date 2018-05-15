//===--------------------------------------------------------------------------------------------===
// orbit/sema/type.c
// This source is part of Orbit - Sema
//
// Created on 2017-10-10 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdio.h>
#include <orbit/ast/ast.h>
#include <orbit/ast/builders.h>
#include <orbit/ast/traversal.h>
#include <orbit/ast/diag.h>
#include <orbit/csupport/tokens.h>
#include <orbit/mangling/mangle.h>
#include <orbit/sema/type.h>
#include <orbit/utils/memory.h>

#include "type_private.h"


/*
Type Analysis strategy

1. Go through type declarations, add to canonical type table.
2. Go through everything that can be typed easily (literals)

x. scoped analysis. build symbol tables, from global to nested scopes

*/

AST* sema_typeCopy(AST* src) {
    if(src == NULL) { return NULL; }
    AST* copy = NULL;
    
    switch (src->kind) {
    case AST_TYPEEXPR_ANY:
    case AST_TYPEEXPR_BOOL:
    case AST_TYPEEXPR_STRING:
    case AST_TYPEEXPR_NUMBER:
    case AST_TYPEEXPR_VOID:
        copy = ast_makePrimitiveType(src->kind);
        break;
    case AST_TYPEEXPR_ARRAY:
        copy = ast_makeArrayType(sema_typeCopy(src->typeExpr.arrayType.elementType));
        break;
    case AST_TYPEEXPR_MAP:
        copy = ast_makeMapType(sema_typeCopy(src->typeExpr.mapType.keyType), 
                               sema_typeCopy(src->typeExpr.mapType.elementType));
        break;
    case AST_TYPEEXPR_FUNC:
        copy = ast_makeFuncType(sema_typeCopy(src->typeExpr.funcType.returnType), 
                                sema_typeCopy(src->typeExpr.funcType.params));
        break;
    case AST_TYPEEXPR_USER:
        copy = ast_makeUserTypePooled(src->typeExpr.userType.symbol);
        break;
    default:
        // TODO: throw error here, we're not working with a type expression.
        fprintf(stderr, "ERROR IN THERE\n");
        break;
    }
    copy->next = src->next ? ORCRETAIN(sema_typeCopy(src->next)) : NULL;
    return copy;
}

bool sema_typeEquals(AST* a, AST* b) {
    if(a == b) { return true; }
    if(!a && !b) { return true; }
    if(!a || !b) { return false; }
    if(!(a->kind & ASTTypeExprMask) || !(b->kind & ASTTypeExprMask)) { return false; }
    
    // TODO: refine, define and implement special `Any` semantics.
    if(a->kind != b->kind) { return false; }
    if(!sema_typeEquals(a->next, b->next)) { return false; }
    
    switch(a->kind) {
    case AST_TYPEEXPR_ANY:
    case AST_TYPEEXPR_BOOL:
    case AST_TYPEEXPR_STRING:
    case AST_TYPEEXPR_NUMBER:
    case AST_TYPEEXPR_VOID:
        return true;
        
    case AST_TYPEEXPR_ARRAY:
        return sema_typeEquals(a->typeExpr.arrayType.elementType,
                               b->typeExpr.arrayType.elementType);
    case AST_TYPEEXPR_MAP:
        return sema_typeEquals(a->typeExpr.mapType.elementType, b->typeExpr.mapType.elementType)
            && sema_typeEquals(a->typeExpr.mapType.keyType, b->typeExpr.mapType.keyType);
    case AST_TYPEEXPR_FUNC:
        return sema_typeEquals(a->typeExpr.funcType.returnType, b->typeExpr.funcType.returnType)
            && sema_typeEquals(a->typeExpr.funcType.params, b->typeExpr.funcType.params);
        
    case AST_TYPEEXPR_USER:
        return a->typeExpr.userType.symbol == b->typeExpr.userType.symbol;
    default:
        break;
    }
    // TODO: add unreachable flag here
    return NULL;
}

void sema_extractTypeAnnotations(AST* decl, void* data) {
    if(!decl->varDecl.typeAnnotation) { return; }
    if(decl->varDecl.typeAnnotation->kind == AST_TYPEEXPR_USER) {
        // If we have a user type we need to make sure it's been declared first
        OCStringID symbol = decl->varDecl.typeAnnotation->typeExpr.userType.symbol;
        
        AST* typeDecl = sema_lookupType(
            (OCSema*)data,
            symbol
        );
        
        if(!typeDecl) {
            OCSourceLoc loc = decl->varDecl.typeAnnotation->sourceRange.start;
            orbit_diagEmitError(loc, "unkown type '$0'", 1, ORBIT_DIAG_STRING(symbol));
            return;
        }
    }
    decl->type = ORCRETAIN(decl->varDecl.typeAnnotation);
}

void sema_installUserTypes(AST* typeDecl, void* data) {
    OCSema* sema = (OCSema*)data;
    OCStringID name = typeDecl->structDecl.name;
    
    if(orbit_rcMapGetP(&sema->typeTable, name)) {
        OCSourceLoc loc = typeDecl->structDecl.symbol.sourceLoc;
        orbit_diagEmitError(loc, "type '$0' was declared before", 1, ORBIT_DIAG_STRING(name));
        
    } else {
        sema_declareType(sema, name, typeDecl);
    }
}

void sema_extractLiteralTypes(AST* literal, void* data) {
    //OCSema* sema = (OCSema*)data;
    switch(literal->kind) {
    case AST_EXPR_CONSTANT_INTEGER:
    case AST_EXPR_CONSTANT_FLOAT:
        literal->type = ORCRETAIN(ast_makePrimitiveType(AST_TYPEEXPR_NUMBER));
        break;
        
    case AST_EXPR_CONSTANT_STRING:
        literal->type = ORCRETAIN(ast_makePrimitiveType(AST_TYPEEXPR_STRING));
        break;
    default:
        // TODO: signal error here, though that shouldn't really happen?
        break;
    }
}

void sema_extractFuncTypes(AST* func, void* data) {
    
    AST* returnType = func->funcDecl.returnType ?
                        func->funcDecl.returnType :
                        ast_makePrimitiveType(AST_TYPEEXPR_VOID);
    
    ASTListBuilder params;
    ast_listStart(&params);
    AST* param = func->funcDecl.params;
    while(param) {
        ast_listAdd(&params, sema_typeCopy(param->type));
        param = param->next;
    }
    func->type = ORCRETAIN(ast_makeFuncType(returnType, ast_listClose(&params)));
    // TODO: We have a problem for overloads here. Do we need to do name mangling first? Or store
    // a collection of functions in the symbol table instead (and do mangling at code generation
    // time, since it's needed for VM operations?)
    func->funcDecl.mangledName = orbit_mangleFuncName(func);
    
    
    sema_declareSymbol((OCSema*)data, func->funcDecl.name, func->type);
}

bool sema_checkExpression(AST* expression, OCSema* sema) {
    return true;
}

void sema_checkBlock(AST* block, OCSema* sema) {
    
}

void sema_doScopeAnalysis(AST* func, void* data) {
    //OCSema* sema = (OCSema*)data;
    // TODO: sema should only contain the global scope right now.
    sema_checkBlock(func->funcDecl.body, (OCSema*)data);
}

void sema_runTypeAnalysis(AST* ast) {
    // Initialise a Sema object
    OCSema sema;
    sema_init(&sema);
    
    ast_traverse(ast, AST_DECL_STRUCT, &sema, &sema_installUserTypes);
    ast_traverse(ast, AST_EXPR_CONSTANT_INTEGER
                    | AST_EXPR_CONSTANT_FLOAT
                    | AST_EXPR_CONSTANT_STRING, &sema, &sema_extractLiteralTypes);
    // Type all the declarations we can type easily
    ast_traverse(ast, AST_DECL_VAR, &sema, &sema_extractTypeAnnotations);
    ast_traverse(ast, AST_DECL_FUNC, &sema, &sema_extractFuncTypes);
    
    ast_traverse(ast, AST_DECL_FUNC, &sema, &sema_doScopeAnalysis);
    
    
    sema_deinit(&sema);
}
