//===--------------------------------------------------------------------------------------------===
// sema.c - Sema helper functions
// This source is part of Orbit - Sema
//
// Created on 2018-04-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/utils/rcmap.h>
#include <orbit/utils/assert.h>
#include "type_private.h"

void sema_init(OCSema* sema) {
    OASSERT(sema != NULL, "Null instance error");
    orbit_rcMapInit(&sema->typeTable);
    
    // Create stack[0], global scope
    sema->stackSize = 1;
    sema->stack[0].parent = NULL;
    orbit_rcMapInit(&sema->stack[0].symbolTable);
}

void sema_deinit(OCSema* sema) {
    OASSERT(sema != NULL, "Null instance error");
    orbit_rcMapDeinit(&sema->typeTable);
    
    sema->stackSize = 0;
    orbit_rcMapDeinit(&sema->stack[0].symbolTable);
}

OCScope* sema_pushScope(OCSema* sema) {
    OASSERT(sema != NULL, "Null instance error");
    OASSERT(sema->stackSize < ORBIT_SEMA_SCOPESTACK_SIZE, "Sema stack overflow");
    
    OCScope* scope = &sema->stack[sema->stackSize];
    scope->parent = &sema->stack[sema->stackSize-1];
    orbit_rcMapInit(&scope->symbolTable);
    sema->stackSize += 1;
    return scope;
}

void sema_popScope(OCSema* sema) {
    OASSERT(sema != NULL, "Null instance error");
    OASSERT(sema->stackSize > 1, "Sema stack underflow");
    sema->stackSize -= 1;
    
    orbit_rcMapDeinit(&sema->stack[sema->stackSize].symbolTable);
    sema->stack[sema->stackSize].parent = NULL;
}

AST* sema_lookupSymbolP(OCSema* sema, OCStringID symbol) {
    OASSERT(sema != NULL, "Null instance error");
    if(sema->stackSize == 0) { return NULL; }
    
    OCScope* scope = &sema->stack[sema->stackSize-1];
    while(scope) {
        AST* result = orbit_rcMapGetP(&scope->symbolTable, symbol);
        if(result) { return result; }
        scope = scope->parent;
    }
    return NULL;
}

void sema_declareSymbol(OCSema* sema, OCStringID symbol, AST* type) {
    OASSERT(sema != NULL, "Null instance error");
    OASSERT(type != NULL, "Null type error");
    OASSERT(sema->stackSize > 0, "Sema no scope error");
    
    OCScope* scope = &sema->stack[sema->stackSize-1];
    orbit_rcMapInsertP(&scope->symbolTable, symbol, type);
}

void sema_declareType(OCSema* sema, OCStringID name, AST* declaration) {
    OASSERT(sema != NULL, "Null instance error");
    OASSERT(declaration != NULL, "Null declaration error");
    
    orbit_rcMapInsertP(&sema->typeTable, name, declaration);
}

AST* sema_lookupType(OCSema* sema, OCStringID name) {
    OASSERT(sema != NULL, "Null instance error");
    return orbit_rcMapGetP(&sema->typeTable, name);
}

static void sema_mangleType(AST* type, OCStringBuffer* buffer) {
    switch (type->kind) {
        case AST_TYPEEXPR_ANY:      orbit_stringBufferAppend(buffer, 'a');  break;
        case AST_TYPEEXPR_BOOL:     orbit_stringBufferAppend(buffer, 'b');  break;
        case AST_TYPEEXPR_STRING:   orbit_stringBufferAppend(buffer, 's');  break;
        case AST_TYPEEXPR_NUMBER:   orbit_stringBufferAppend(buffer, 'd');  break;
        case AST_TYPEEXPR_VOID:     orbit_stringBufferAppend(buffer, 'v');  break;
        case AST_TYPEEXPR_NIL:                                              break;
            
        case AST_TYPEEXPR_ARRAY:
            orbit_stringBufferAppendC(buffer, "%a[", 3);
            sema_mangleType(type->typeExpr.arrayType.elementType, buffer);
            orbit_stringBufferAppendC(buffer, "]", 1);
            break;
        case AST_TYPEEXPR_MAP:
            orbit_stringBufferAppendC(buffer, "%m[", 3);
            sema_mangleType(type->typeExpr.mapType.keyType, buffer);
            orbit_stringBufferAppendC(buffer, "#", 1);
            sema_mangleType(type->typeExpr.mapType.elementType, buffer);
            orbit_stringBufferAppendC(buffer, "]", 1);
            break;
            
        case AST_TYPEEXPR_FUNC:
            orbit_stringBufferAppendC(buffer, "%f[", 3);
            sema_mangleType(type->typeExpr.funcType.params, buffer);
            orbit_stringBufferAppendC(buffer, "#", 1);
            sema_mangleType(type->typeExpr.funcType.returnType, buffer);
            orbit_stringBufferAppendC(buffer, "]", 1);
            break;
            
        case AST_TYPEEXPR_USER:
            orbit_stringBufferAppendC(buffer, "%u[", 3);
            OCString* name = orbit_stringPoolGet(type->typeExpr.userType.symbol);
            orbit_stringBufferAppendC(buffer, name->data, name->length);
            orbit_stringBufferAppendC(buffer, "]", 1);
            break;
        default:
            break;
    }
    
    if(type->next) {
        orbit_stringBufferAppend(buffer, '_');
        sema_mangleType(type->next, buffer);
    }
}

OCStringID sema_mangleFuncName(AST* decl) {
    OCStringBuffer buffer;
    orbit_stringBufferInit(&buffer, 128);
    
    OCString* name = orbit_stringPoolGet(decl->funcDecl.name);
    
    orbit_stringBufferAppend(&buffer, '@');
    orbit_stringBufferAppendC(&buffer, name->data, name->length);
    
    // Now do the params, probably the most tedious part of all this
    AST* params = decl->type->typeExpr.funcType.params;
    if(params) {
        orbit_stringBufferAppendC(&buffer, "__", 2);
        sema_mangleType(params, &buffer);
    }
    
    OCStringID id = orbit_stringBufferIntern(&buffer);
    orbit_stringBufferDeinit(&buffer);
    return id;
}
