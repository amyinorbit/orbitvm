//===--------------------------------------------------------------------------------------------===
// orbit/sema/type.c
// This source is part of Orbit - Sema
//
// Created on 2017-10-10 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/source/tokens.h>
#include <orbit/utils/memory.h>
#include <orbit/ast/ast.h>
#include <orbit/ast/traversal.h>
#include <orbit/sema/type.h>


/*
Type Analysis strategy

1. Go through type declarations, add to canonical type table.
2. Go through everything that can be typed easily (literals)

x. scoped analysis. build symbol tables, from global to nested scopes

*/

bool sema_typeEquals(AST* a, AST* b) {
    if(a == b) { return true; }
    if(a == NULL || b == NULL) { return false; }
    if(!(a->kind & ASTTypeExprMask) || !(b->kind & ASTTypeExprMask)) { return false; }
    // TODO: refine, define and implement special `Any` semantics.
    
    a = a->typeExpr.canonicalType;
    b = b->typeExpr.canonicalType;
    if(a->kind != b->kind) { return false; }
    
    switch(a->kind) {
    case AST_TYPEEXPR_ANY:
    case AST_TYPEEXPR_BOOL:
    case AST_TYPEEXPR_STRING:
    case AST_TYPEEXPR_NUMBER:
        return true;
        
    case AST_TYPEEXPR_ARRAY:
        return sema_typeEquals(a->typeExpr.arrayType.elementType,
                               b->typeExpr.arrayType.elementType);
    case AST_TYPEEXPR_MAP:
        return sema_typeEquals(a->typeExpr.mapType.elementType, b->typeExpr.mapType.elementType)
            && sema_typeEquals(a->typeExpr.mapType.keyType, b->typeExpr.mapType.keyType);
    case AST_TYPEEXPR_FUNC:
        if(!sema_typeEquals(a->typeExpr.funcType.returnType, b->typeExpr.funcType.returnType)) {
            return false;
        }
        // TODO: compare argument list
        break;
        
    case AST_TYPEEXPR_USER:
        // TODO: compare token name
        break;
    default:
        break;
    }
    
    return NULL;
}

void sema_extractVariableTypes(AST* ast, void* data) {
    
}

void sema_extractFunctionTypes(AST* ast, void* data) {
    
}

void sema_extractLiteralTypes(AST* ast, void* data) {
    switch(ast->kind) {
    case AST_EXPR_CONSTANT_INTEGER:
    case AST_EXPR_CONSTANT_FLOAT:
        //ast->type = ORCRETAIN(ast_makeTypeExpr(AST_TYPEEXPR_NUMBER));
        break;
    case AST_EXPR_CONSTANT_STRING:
        //ast->type = ORCRETAIN(ast_makeTypeExpr(AST_TYPEEXPR_STRING));
        break;
    default:
        break;
    }
}

void sema_extractCallTypes(AST* ast, void* data) {
    // TODO: implementation
}

void sema_extractNameTypes(AST* ast, void* data) {
    // TODO: implementation
}

void sema_extractUnaryTypes(AST* ast, void* data) {
    // TODO: implementation
}

void sema_extractBinaryTypes(AST* ast, void* data) {
   // TODO: implementation
}

void sema_inferVariableTypes(AST* ast, void* data) {
    // TODO: implementation
}

void sema_runTypeAnalysis(AST* ast) {
    ast_traverse(ast,
                 AST_EXPR_CONSTANT_INTEGER | AST_EXPR_CONSTANT_FLOAT | AST_EXPR_CONSTANT_STRING,
                 NULL, &sema_extractLiteralTypes);
    ast_traverse(ast, AST_DECL_VAR, NULL,  &sema_extractVariableTypes);
    ast_traverse(ast, AST_DECL_FUNC, NULL, &sema_extractFunctionTypes);
}
