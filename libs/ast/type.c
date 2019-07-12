//===--------------------------------------------------------------------------------------------===
// orbit/ast/type.c - Implementation of the compile-time Orbit type system
// This source is part of Orbit - AST
//
// Created on 2018-05-15 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/ast/builders.h>
#include <orbit/ast/type.h>
#include <orbit/csupport/string.h>
#include <assert.h>
#include <stdio.h>

bool orbitASTTypeIsPrimitive(const OrbitAST* a) {
    return (a && (a->kind & ASTPrimitiveMask));
}

bool orbitASTTypeEqualsPrimitive(const OrbitAST* a, ASTKind b) {
    assert((b & ASTPrimitiveMask) && "Cannot compare to a non primitive type");
    if(!orbitASTTypeIsPrimitive(a)) return false;
    return a->kind == b;
}

static bool typeEqualsImpl(const OrbitAST* a, const OrbitAST* b, bool inList) {
    if(a == b) { return true; }
    if(!a && !b) { return true; }
    if(!a || !b) { return false; }
    if(!(a->kind & ASTTypeExprMask) || !(b->kind & ASTTypeExprMask)) { return false; }
    
    // TODO: refine, define and implement special `Any` semantics.
    if(a->kind != b->kind) { return false; }
    if(inList && !typeEqualsImpl(a->next, b->next, true)) { return false; }
    
    switch(a->kind) {
    case ORBIT_AST_TYPEEXPR_ANY:
    case ORBIT_AST_TYPEEXPR_BOOL:
    case ORBIT_AST_TYPEEXPR_STRING:
    case ORBIT_AST_TYPEEXPR_INT:
    case ORBIT_AST_TYPEEXPR_FLOAT:
    case ORBIT_AST_TYPEEXPR_VOID:
        return true;
        
    case ORBIT_AST_TYPEEXPR_ARRAY:
        return typeEqualsImpl(a->typeExpr.arrayType.elementType,
                              b->typeExpr.arrayType.elementType,
                              false);
    case ORBIT_AST_TYPEEXPR_MAP:
        return typeEqualsImpl(a->typeExpr.mapType.elementType, b->typeExpr.mapType.elementType, false)
            && typeEqualsImpl(a->typeExpr.mapType.keyType, b->typeExpr.mapType.keyType, false);
    case ORBIT_AST_TYPEEXPR_FUNC:
        return typeEqualsImpl(a->typeExpr.funcType.returnType, b->typeExpr.funcType.returnType, false)
            && typeEqualsImpl(a->typeExpr.funcType.params, b->typeExpr.funcType.params, true);
        
    case ORBIT_AST_TYPEEXPR_USER:
        return a->typeExpr.userType.symbol == b->typeExpr.userType.symbol;
    default:
        break;
    }
    // TODO: add unreachable flag here
    return NULL;
}

bool orbitASTTypeEquals(const OrbitAST* a, const OrbitAST* b) {
    return typeEqualsImpl(a, b, false);
}

OrbitAST* orbitASTTypeCopy(const OrbitAST* src) {
    if(src == NULL) { return NULL; }
    OrbitAST* copy = NULL;
    
    switch (src->kind) {
    case ORBIT_AST_TYPEEXPR_ANY:
    case ORBIT_AST_TYPEEXPR_BOOL:
    case ORBIT_AST_TYPEEXPR_STRING:
    case ORBIT_AST_TYPEEXPR_INT:
    case ORBIT_AST_TYPEEXPR_FLOAT:
    case ORBIT_AST_TYPEEXPR_VOID:
        copy = orbitASTMakePrimitiveType(src->kind);
        break;
    case ORBIT_AST_TYPEEXPR_ARRAY:
        copy = orbitASTMakeArrayType(orbitASTTypeCopy(src->typeExpr.arrayType.elementType));
        break;
    case ORBIT_AST_TYPEEXPR_MAP:
        copy = orbitASTMakeMapType(orbitASTTypeCopy(src->typeExpr.mapType.keyType), 
                                    orbitASTTypeCopy(src->typeExpr.mapType.elementType));
        break;
    case ORBIT_AST_TYPEEXPR_FUNC:
        copy = orbitASTMakeFuncType(orbitASTTypeCopy(src->typeExpr.funcType.returnType), 
                                     orbitASTTypeCopy(src->typeExpr.funcType.params));
        break;
    case ORBIT_AST_TYPEEXPR_USER:
        copy = orbitASTMakeUserTypePooled(src->typeExpr.userType.symbol);
        break;
    default:
        // TODO: throw error here, we're not working with a type expression.
        fprintf(stderr, "UNREACHEABLE\n");
        break;
    }
    copy->next = src->next ? ORCRETAIN(orbitASTTypeCopy(src->next)) : NULL;
    copy->typeExpr.flags = src->typeExpr.flags;
    return copy;
}

void orbitASTTypeString(OCStringBuffer* buffer, const OrbitAST* ast) {
    if(ast == NULL) {
        orbitStringBufferAppendC(buffer, "()", 2);
        return;
    }
    if((ast->kind & ASTTypeExprMask) == 0) { return; }
    
    if((ast->typeExpr.flags & ORBIT_TYPE_OPTIONAL)) {
        orbitStringBufferAppendC(buffer, "maybe ", 6);
    }
    
    switch(ast->kind) {
    case ORBIT_AST_TYPEEXPR_VOID:     orbitStringBufferAppendC(buffer, "()", 2);       break;
    case ORBIT_AST_TYPEEXPR_BOOL:     orbitStringBufferAppendC(buffer, "Bool", 4);     break;
    case ORBIT_AST_TYPEEXPR_INT:      orbitStringBufferAppendC(buffer, "Int", 3);      break;
    case ORBIT_AST_TYPEEXPR_FLOAT:    orbitStringBufferAppendC(buffer, "Float", 5);    break;
    case ORBIT_AST_TYPEEXPR_STRING:   orbitStringBufferAppendC(buffer, "String", 6);   break;
    case ORBIT_AST_TYPEEXPR_ANY:      orbitStringBufferAppendC(buffer, "Any", 3);      break;
    case ORBIT_AST_TYPEEXPR_USER:
        orbitStringBufferAppendP(buffer, ast->typeExpr.userType.symbol);
        break;
        
    case ORBIT_AST_TYPEEXPR_FUNC:
        // orbitStringBufferAppend(buffer, '(');
        orbitASTTypeString(buffer, ast->typeExpr.funcType.params);
        orbitStringBufferAppendC(buffer, " -> ", 4);
        orbitASTTypeString(buffer, ast->typeExpr.funcType.returnType);
        break;
        
    case ORBIT_AST_TYPEEXPR_ARRAY:
        orbitStringBufferAppendC(buffer, "Array<", 6);
        orbitASTTypeString(buffer, ast->typeExpr.arrayType.elementType);
        orbitStringBufferAppend(buffer, '>');
        break;
        
    case ORBIT_AST_TYPEEXPR_MAP:
        orbitStringBufferAppendC(buffer, "Map<", 4);
        orbitASTTypeString(buffer, ast->typeExpr.mapType.keyType);
        orbitStringBufferAppend(buffer, ':');
        orbitASTTypeString(buffer, ast->typeExpr.mapType.elementType);
        orbitStringBufferAppend(buffer, '>');
        break;
        
    default:
        break;
    }
    
    if(ast->next) {
        orbitStringBufferAppendC(buffer, ", ", 2);
        orbitASTTypeString(buffer, ast->next);
    }
}