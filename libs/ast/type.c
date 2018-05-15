//===--------------------------------------------------------------------------------------------===
// orbit/ast/type.c - Implementation of the compile-time Orbit type system
// This source is part of Orbit - AST
//
// Created on 2018-05-15 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/ast/type.h>
#include <orbit/csupport/string.h>

#include <stdio.h>

void orbit_astTypeString(OCStringBuffer* buffer, OrbitAST* ast) {
    if(ast == NULL) { return; }
    if((ast->kind & ASTTypeExprMask) == 0) { return; }
    
    
    switch(ast->kind) {
    case ORBIT_AST_TYPEEXPR_VOID:     orbit_stringBufferAppendC(buffer, "Void", 4);   break;
    case ORBIT_AST_TYPEEXPR_BOOL:     orbit_stringBufferAppendC(buffer, "Bool", 4);   break;
    case ORBIT_AST_TYPEEXPR_NUMBER:   orbit_stringBufferAppendC(buffer, "Number", 6); break;
    case ORBIT_AST_TYPEEXPR_STRING:   orbit_stringBufferAppendC(buffer, "String", 6); break;
    case ORBIT_AST_TYPEEXPR_ANY:      orbit_stringBufferAppendC(buffer, "Any", 3);    break;
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
        orbit_stringBufferAppendC(buffer, "Array[", 6);
        orbit_astTypeString(buffer, ast->typeExpr.arrayType.elementType);
        orbit_stringBufferAppend(buffer, ']');
        break;
        
    case ORBIT_AST_TYPEEXPR_MAP:
        orbit_stringBufferAppendC(buffer, "Map[", 4);
        orbit_astTypeString(buffer, ast->typeExpr.mapType.keyType);
        orbit_stringBufferAppend(buffer, ':');
        orbit_astTypeString(buffer, ast->typeExpr.mapType.elementType);
        orbit_stringBufferAppend(buffer, ']');
        break;
        
    default:
        break;
    }
    
    if(ast->next) {
        orbit_stringBufferAppendC(buffer, ", ", 2);
        orbit_astTypeString(buffer, ast->next);
    }
}