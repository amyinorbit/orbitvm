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

#include <stdio.h>

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

bool orbit_astTypeEquals(const OrbitAST* a, const OrbitAST* b) {
    return typeEqualsImpl(a, b, false);
}

OrbitAST* orbit_astTypeCopy(const OrbitAST* src) {
    if(src == NULL) { return NULL; }
    OrbitAST* copy = NULL;
    
    switch (src->kind) {
    case ORBIT_AST_TYPEEXPR_ANY:
    case ORBIT_AST_TYPEEXPR_BOOL:
    case ORBIT_AST_TYPEEXPR_STRING:
    case ORBIT_AST_TYPEEXPR_INT:
    case ORBIT_AST_TYPEEXPR_FLOAT:
    case ORBIT_AST_TYPEEXPR_VOID:
        copy = orbit_astMakePrimitiveType(src->kind);
        break;
    case ORBIT_AST_TYPEEXPR_ARRAY:
        copy = orbit_astMakeArrayType(orbit_astTypeCopy(src->typeExpr.arrayType.elementType));
        break;
    case ORBIT_AST_TYPEEXPR_MAP:
        copy = orbit_astMakeMapType(orbit_astTypeCopy(src->typeExpr.mapType.keyType), 
                                    orbit_astTypeCopy(src->typeExpr.mapType.elementType));
        break;
    case ORBIT_AST_TYPEEXPR_FUNC:
        copy = orbit_astMakeFuncType(orbit_astTypeCopy(src->typeExpr.funcType.returnType), 
                                     orbit_astTypeCopy(src->typeExpr.funcType.params));
        break;
    case ORBIT_AST_TYPEEXPR_USER:
        copy = orbit_astMakeUserTypePooled(src->typeExpr.userType.symbol);
        break;
    default:
        // TODO: throw error here, we're not working with a type expression.
        fprintf(stderr, "UNREACHEABLE\n");
        break;
    }
    copy->next = src->next ? ORCRETAIN(orbit_astTypeCopy(src->next)) : NULL;
    copy->typeExpr.flags = src->typeExpr.flags;
    return copy;
}

void orbit_astTypeString(OCStringBuffer* buffer, OrbitAST* ast) {
    if(ast == NULL) { return; }
    if((ast->kind & ASTTypeExprMask) == 0) { return; }
    
    if((ast->typeExpr.flags & ORBIT_TYPE_OPTIONAL)) {
        orbit_stringBufferAppendC(buffer, "maybe ", 6);
    }
    
    switch(ast->kind) {
    case ORBIT_AST_TYPEEXPR_VOID:     orbit_stringBufferAppendC(buffer, "Void", 4);     break;
    case ORBIT_AST_TYPEEXPR_BOOL:     orbit_stringBufferAppendC(buffer, "Bool", 4);     break;
    case ORBIT_AST_TYPEEXPR_INT:      orbit_stringBufferAppendC(buffer, "Int", 3);      break;
    case ORBIT_AST_TYPEEXPR_FLOAT:    orbit_stringBufferAppendC(buffer, "Float", 5);    break;
    case ORBIT_AST_TYPEEXPR_STRING:   orbit_stringBufferAppendC(buffer, "String", 6);   break;
    case ORBIT_AST_TYPEEXPR_ANY:      orbit_stringBufferAppendC(buffer, "Any", 3);      break;
    case ORBIT_AST_TYPEEXPR_USER:
        orbit_stringBufferAppendP(buffer, ast->typeExpr.userType.symbol);
        break;
        
    case ORBIT_AST_TYPEEXPR_FUNC:
        orbit_stringBufferAppend(buffer, '(');
        orbit_astTypeString(buffer, ast->typeExpr.funcType.params);
        orbit_stringBufferAppendC(buffer, ") -> ", 5);
        orbit_astTypeString(buffer, ast->typeExpr.funcType.returnType);
        break;
        
    case ORBIT_AST_TYPEEXPR_ARRAY:
        orbit_stringBufferAppendC(buffer, "Array<", 6);
        orbit_astTypeString(buffer, ast->typeExpr.arrayType.elementType);
        orbit_stringBufferAppend(buffer, '>');
        break;
        
    case ORBIT_AST_TYPEEXPR_MAP:
        orbit_stringBufferAppendC(buffer, "Map<", 4);
        orbit_astTypeString(buffer, ast->typeExpr.mapType.keyType);
        orbit_stringBufferAppend(buffer, ':');
        orbit_astTypeString(buffer, ast->typeExpr.mapType.elementType);
        orbit_stringBufferAppend(buffer, '>');
        break;
        
    default:
        break;
    }
    
    if(ast->next) {
        orbit_stringBufferAppendC(buffer, ", ", 2);
        orbit_astTypeString(buffer, ast->next);
    }
}