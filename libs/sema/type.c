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
#include <orbit/source/tokens.h>
#include <orbit/utils/memory.h>
#include <orbit/ast/ast.h>
#include <orbit/ast/builders.h>
#include <orbit/ast/traversal.h>
#include <orbit/sema/type.h>

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
    case AST_TYPEEXPR_NIL:
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
        copy = ast_makeUserType(&src->typeExpr.userType.symbol);
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
    case AST_TYPEEXPR_NIL:
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
        return source_tokenEquals(&a->typeExpr.userType.symbol, &b->typeExpr.userType.symbol);
    default:
        // TODO: throw sema type error
        break;
    }
    // TODO: add unreachable flag here
    return NULL;
}

void sema_extractTypeAnnotations(AST* decl, void* data) {
    if(!decl->varDecl.typeAnnotation) { return; }
    // TODO: Type check first please
    decl->type = ORCRETAIN(decl->varDecl.typeAnnotation);
}

void sema_installUserTypes(AST* typeDecl, void* data) {
    OCSema* sema = (OCSema*)data;
    OCToken tok = typeDecl->structDecl.symbol;
    UTFConstString* symbol = source_stringFromToken(&tok);
    ORCRETAIN(symbol);
    
    if(orbit_rcMapGet(&sema->typeTable, symbol)) {
        // A type already exists under that name, error!
        fprintf(stderr, "error: A type already exists under that name\n");
    } else {
        // TODO: need much better sema error reporting. Will come with AST printing and Diag
        fprintf(stderr, "Found a user-defined type: '%.*s'\n", (int)symbol->length, symbol->data);
        orbit_rcMapInsert(&sema->typeTable, symbol, typeDecl);
        //orbit_rcArrayAppend(&sema->uniqueTypes, typeDecl);
    }
    ORCRELEASE(symbol);
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
    func->type = ORCRETAIN(ast_makeFuncType(returnType, 
                                            ast_listClose(&params)));
}

void sema_doScopeAnalysis(AST* function, void* data) {
    //OCSema* sema = (OCSema*)data;
    // TODO: sema should only contain the global scope right now.
}

void sema_runTypeAnalysis(AST* ast) {
    fprintf(stderr, "Running Sema Type Analysis\n");
    
    
    // Initialise a Sema object
    OCSema sema;
    ORCRETAIN(orbit_rcMapInit(&sema.typeTable));
    ORCRETAIN(orbit_rcArrayInit(&sema.uniqueTypes, 64));
    
    ast_traverse(ast, AST_DECL_STRUCT, &sema, &sema_installUserTypes);
    ast_traverse(ast, AST_EXPR_CONSTANT_INTEGER
                    | AST_EXPR_CONSTANT_FLOAT
                    | AST_EXPR_CONSTANT_STRING, &sema, &sema_extractLiteralTypes);
    // Type all the declarations we can type easily
    ast_traverse(ast, AST_DECL_VAR, &sema, &sema_extractTypeAnnotations);
    // ast_traverse(ast, AST_DECL_VAR, &sema,  &sema_extractVariableTypes);
    ast_traverse(ast, AST_DECL_FUNC, &sema, &sema_extractFuncTypes);
    
    orbit_rcMapDeinit(&sema.typeTable);
    orbit_rcArrayDeinit(&sema.uniqueTypes);
}
