//===--------------------------------------------------------------------------------------------===
// helpers.c - Implemnentation of bookkeeping Sema functions
// This source is part of Orbit
//
// Created on 2019-07-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/sema/typecheck.h>
#include <orbit/utils/memory.h>
#include <orbit/ast/type.h>
#include <orbit/compiler.h>
#include <assert.h>
#include "helpers.h"
#include "errors.h"


void orbitSemaInit(Sema* self) {
    assert(self && "null semantic checker error");
    self->context = NULL;
    self->current = NULL;
    resolverInit(&self->resolver);
}

void orbitSemaDeinit(Sema* self) {
    assert(self && "null semantic checker error");
    resolverDeinit(&self->resolver);
}

void pushScope(Sema* self, OCScope* scope) {
    orbitScopeInit(scope);
    scope->parent = self->current;
    self->current = scope;
}

void popScope(Sema* self) {
    // TODO: we should assert here instead
    if(!self->current) return;
    OCScope* popped = self->current;
    self->current = popped->parent;
    orbitScopeDeinit(popped);
}

static OCScope* currentScope(Sema* self) {
    return self->current;
}

static OCScope* parentScope(Sema* self) {
    assert(self->current->parent && "already at the global scope");
    return self->current->parent;
}

const OCSymbol* lookupSymbol(Sema* self, OCStringID name) {
    assert(self && "null semantic checker error");
    OCScope* scope = currentScope(self);
    while(scope) {
        const OCSymbol* decl = orbitSymbolLookup(scope, name);
        if(decl) return decl;
        scope = scope->parent;
    }
    return NULL;
}

bool declareFunction(Sema* self, OrbitAST* decl) {
    assert(self && "null semantic checker error");
    assert(decl && "null function decl error");
    assert(decl->kind == ORBIT_AST_DECL_FUNC && "invalid function type declaration");
    
    // When we declare a function, we've already extracted the parameters and opened a scope for
    // the function. We need to declare the function in the scope that contains it, so one scope
    // up.
    OCScope* scope = parentScope(self);
    OCStringID name = decl->funcDecl.name;
    
    // For a function declaration to be valid, its symbol must be either not present in the table,
    // or it its parameter list must be different (overload)
    const OCSymbol* conflicting = orbitFunctionDeclare(scope, name, decl);
    if(!conflicting) return true;
    errorAlreadyDeclared(self, decl, conflicting->decl);
    return false;
}

bool declareVariable(Sema* self, OrbitAST* decl) {
    assert(self && "null semantic checker error");
    assert(decl && "null function decl error");
    assert(decl->kind == ORBIT_AST_DECL_VAR && "invalid variable type declaration");
    
    OCScope* scope = currentScope(self);
    OCStringID name = decl->varDecl.name;
    
    const OCSymbol* existing = orbitSymbolDeclare(scope, name, decl);
    if(!existing) return true;
    
    errorAlreadyDeclared(self, decl, existing->decl);
    return false;
}
