//
//  orbit/sema/type.c
//  Orbit - Sema
//
//  Created by Amy Parent on 2017-10-10.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <orbit/source/tokens.h>
#include <orbit/utils/memory.h>
#include <orbit/type/type.h>
#include <orbit/type/builders.h>
#include <orbit/ast/traversal.h>
#include <orbit/sema/type.h>

static Type* sema_extractSimpleType(OCToken token) {
    switch(token.kind) {
    case TOKEN_NIL:
        return type_make(TYPE_NIL, false);
    case TOKEN_ANY:
        return type_make(TYPE_ANY, false);
    case TOKEN_BOOL:
        return type_make(TYPE_BOOL, false);
    case TOKEN_NUMBER:
        return type_make(TYPE_NUMBER, false);
    case TOKEN_STRING:
        return type_make(TYPE_STRING, false);
    case TOKEN_VOID:
        return type_make(TYPE_VOID, false);
    default:
        // TODO: throw error, invalid token
        break;
    }
    return NULL;
}

static Type* sema_extractFuncType(AST* ast) {
    Type* returnType = sema_extractType(ast->funcType.returnType);
    Type* paramList = NULL;
    Type** next = &paramList;
    AST* param = ast->funcType.params;
    
    while(param != NULL) {
        Type* t = sema_extractType(param);
        *next = t;
        next = &(t->next);
        param = param->next;
    }
    
    return type_makeFunction(returnType, paramList);
}

Type* sema_extractType(AST* ast) {
    if(ast == NULL) { return NULL; }
    
    switch(ast->kind) {
    case AST_TYPEEXPR_NIL:
    case AST_TYPEEXPR_VOID:
    case AST_TYPEEXPR_BOOL:
    case AST_TYPEEXPR_NUMBER:
    case AST_TYPEEXPR_STRING:
    case AST_TYPEEXPR_USER:
    case AST_TYPEEXPR_ANY:
        return sema_extractSimpleType(ast->simpleType.symbol);
    case AST_TYPEEXPR_ARRAY:
        return type_makeArray(sema_extractType(ast->arrayType.elementType));
    case AST_TYPEEXPR_MAP:
        return type_makeMap(sema_extractType(ast->mapType.keyType),
                            sema_extractType(ast->mapType.elementType));
    case AST_TYPEEXPR_FUNC:
        return sema_extractFuncType(ast);
    default:
        // TODO: Throw error here
        break;
    }
    return NULL;
}

void sema_extractVariableTypes(AST* ast, void* data) {
    if(ast->varDecl.typeAnnotation == NULL) { return; }
    ast->type = sema_extractType(ast->varDecl.typeAnnotation);
}

void sema_extractFunctionTypes(AST* ast, void* data) {
    Type* returnType = NULL;
    if(ast->funcDecl.returnType == NULL) {
        returnType = type_make(TYPE_VOID, false);
    } else {
        returnType = sema_extractType(ast->funcDecl.returnType);
    }
    
    AST* param = ast->funcDecl.params;
    Type* paramList = NULL;
    Type** next = &paramList;
    
    while(param != NULL) {
        Type* t = type_copy(param->type);
        if(t != NULL) {
            *next = t;
            next = &(t->next);
        }
        param = param->next;
    }
    ast->type = type_makeFunction(returnType, paramList);
}

void sema_extractLiteralTypes(AST* ast, void* data) {
    switch(ast->kind) {
    case AST_EXPR_CONSTANT_INTEGER:
    case AST_EXPR_CONSTANT_FLOAT:
        ast->type = type_make(TYPE_NUMBER, true);
        break;
    case AST_EXPR_CONSTANT_STRING:
        ast->type = type_make(TYPE_STRING, true);
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


