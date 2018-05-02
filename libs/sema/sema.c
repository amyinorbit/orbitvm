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
