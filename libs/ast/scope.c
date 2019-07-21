//===--------------------------------------------------------------------------------------------===
// scope.c - Implementation of scope tracking
// This source is part of Orbit
//
// Created on 2019-07-14 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/ast/scope.h>
#include <orbit/ast/ast.h>
#include <orbit/ast/type.h>
#include <assert.h>

void orbitScopeInit(OCScope* scope) {
    assert(scope && "cannot initialise a null scope");
    scope->parent = NULL;
    scope->count = 0;
    scope->capacity = 0;
    scope->symbols = NULL;
}

void orbitScopeDeinit(OCScope* scope) {
    assert(scope && "cannot deinitialise a null scope");
    for(int i = 0; i < scope->count; ++i) {
        ORCRELEASE(scope->symbols[i].decl);
    }
    orbitAllocator(scope->symbols, scope->capacity * sizeof(OCSymbol), 0);
    orbitScopeInit(scope);
}

static void scopeEnsure(OCScope* scope, int required) {
    if(required < scope->capacity) return;
    int oldCapacity = scope->capacity;
    while(required > scope->capacity)
        scope->capacity = ORBIT_GROW_CAPACITY(scope->capacity);
    scope->symbols = ORBIT_REALLOC_ARRAY(scope->symbols, OCSymbol, oldCapacity, scope->capacity);
}

static const OCSymbol* lookupSym(const OCScope* scope, OCStringID name) {
    for(int i = 0; i < scope->count; ++i) {
        if(scope->symbols[i].name == name) return &scope->symbols[i];
    }
    return NULL;
}

const OCSymbol* orbitSymbolLookup(const OCScope* scope, OCStringID name) {
    assert(scope && "cannot lookup a symbol in a null scope");
    do {
        const OCSymbol* sym = lookupSym(scope, name);
        if(sym) return sym;
        scope = scope->parent;
    } while(scope);
    return NULL;
}

static inline int symbolIndex(const OCScope* scope) {
    int index = 0;
    for(int i = 0; i < scope->count; ++i) {
        if(scope->symbols[i].isVariable) index += 1;
    }
    return index;
}

const OCSymbol* orbitSymbolDeclare(OCScope* scope, OCStringID name, const OrbitAST* decl) {
    assert(scope && "cannot declare a symbol in a null scope");
    
    for(int i = 0; i < scope->count; ++i) {
        if(scope->symbols[i].name == name) return &scope->symbols[i];
    }
    scopeEnsure(scope, scope->count + 1);
    scope->symbols[scope->count].name = name;
    scope->symbols[scope->count].index = symbolIndex(scope);
    scope->symbols[scope->count].isVariable = true;
    scope->symbols[scope->count].decl = ORCRETAIN(decl);
    scope->count += 1;
    return NULL;
}

static const OCSymbol* lookupFun(const OCScope* scope, OCStringID name, const OrbitAST* args) {
    for(int i = 0; i < scope->count; ++i) {
        if(scope->symbols[i].name != name) continue;
        const OrbitAST* T = scope->symbols[i].decl->type;
        if(!orbitTypeIsCallable(T)) continue;
        if(!orbitTypeCanCall(T, args)) continue;
        
        return &scope->symbols[i];
    }
    return NULL;
}

// This is a bit dodge - we treat local variables as if they're functions declared as module-level.
// Realistically we probably want a function table separate from the symbols table.
const OCSymbol* orbitFunctionLookup(const OCScope* scope, OCStringID name, const OrbitAST* args) {
    assert(scope && "cannot lookup a symbol in a null scope");
    do {
        const OCSymbol* sym = lookupFun(scope, name, args);
        if(sym) return sym;
        scope = scope->parent;
    } while(scope);
    return NULL;
}

static const OCSymbol* findOverload(const OCScope* scope, OCStringID name, const OrbitAST* decl) {
    for(int i = 0; i < scope->count; ++i) {
        const OCSymbol* other = &scope->symbols[i];
        if(name != other->name) continue;
        if(other->isVariable) continue;
        if(orbitTypesSameOverload(decl->type, other->decl->type)) return other;
    }
    return NULL;
}

const OCSymbol* orbitFunctionDeclare(OCScope* scope, OCStringID name, const OrbitAST* decl) {
    assert(scope && "cannot declare a symbol in a null scope");
    const OCSymbol* existing = findOverload(scope, name, decl);
    if(existing) return existing;
    scopeEnsure(scope, scope->count + 1);
    scope->symbols[scope->count].name = name;
    scope->symbols[scope->count].index = -1;
    scope->symbols[scope->count].isVariable = false;
    scope->symbols[scope->count].decl = ORCRETAIN(decl);
    scope->count += 1;
    return NULL;
}
