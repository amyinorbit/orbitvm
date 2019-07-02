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
#include <assert.h>
#include "helpers.h"
#include "errors.h"

void symbolDeinit(void* ptr) {
    Symbol* symbol = (Symbol*)ptr;
    ORCRELEASE(symbol->decl);
    ORCRELEASE(symbol->next);
}

void orbit_semaInit(Sema* self) {
    assert(self && "null semantic checker error");
    initScope(&self->global, NULL);
    self->context = NULL;
    self->current = self->stack;
    resolverInit(&self->resolver);
}

void orbit_semaDeinit(Sema* self) {
    assert(self && "null semantic checker error");
    while(self->current > self->stack) {
        deinitScope(self->current);
        self->current -= 1;
    }
    deinitScope(&self->global);
    resolverDeinit(&self->resolver);
}

void initScope(Scope* self, Scope* parent) {
    self->parent = parent;
    orbit_rcMapInit(&self->types);
    orbit_rcMapInit(&self->symbols);
}

void deinitScope(Scope* self) {
    orbit_rcMapDeinit(&self->types);
    orbit_rcMapDeinit(&self->symbols);
}

Scope* pushScope(Sema* self) {
    Scope* scope = self->current++;
    Scope* parent = scope == self->stack ? &self->global : (scope-1);
    initScope(scope, parent);
    return scope;
}

void popScope(Sema* self) {
    Scope* scope = (--self->current);
    deinitScope(scope);
}

static Scope* currentScope(Sema* self) {
    return self->current == self->stack
        ? &self->global
        : (self->current-1);
}

static inline OrbitAST* funcParamTypes(OrbitAST* node) {
    return node->type->typeExpr.funcType.params;
}

OrbitAST* findOverload(Symbol* symbol, OrbitAST* params) {
    while(symbol) {
        if(orbit_astTypeEquals(funcParamTypes(symbol->decl), params)) return symbol->decl;
        symbol = symbol->next;
    }
    return NULL;
}

Symbol* lookupSymbol(Sema* self, OCStringID name) {
    assert(self && "null semantic checker error");
    Scope* scope = currentScope(self);
    while(scope) {
        Symbol* decl = orbit_rcMapGetP(&scope->symbols, name);
        if(decl) return decl;
        scope = scope->parent;
    }
    return NULL;
}

bool declareFunction(Sema* self, OrbitAST* decl) {
    assert(self && "null semantic checker error");
    assert(decl && "null function decl error");
    assert(decl->kind == ORBIT_AST_DECL_FUNC && "invalid function type declaration");
    
    Scope* scope = currentScope(self);
    OCStringID name = decl->funcDecl.name;
    
    // For a function declaration to be valid, its symbol must be either not present in the table,
    // or it its parameter list must be different (overload)
    Symbol* existing = orbit_rcMapGetP(&scope->symbols, name);
    Symbol* func = NULL;
    
    if(existing) {
        OrbitAST* conflicting = findOverload(existing, funcParamTypes(decl));
        // We compare the types in the parameter list. 
        if(conflicting) {
            errorAlreadyDeclared(self, decl, conflicting);
            return false;
        }
        
        if(existing->kind != SYM_FUNCTION) {
            // TODO: throw error here
            return false;
        }
        
        func = ORCINIT(ORBIT_ALLOC(Symbol), &symbolDeinit);
        func->next = existing->next;
        existing->next = ORCRETAIN(func);
        
    } else {
        func = ORCINIT(ORBIT_ALLOC(Symbol), &symbolDeinit);
        func->next = NULL;
        orbit_rcMapInsertP(&scope->symbols, name, func);
    }
    func->decl = ORCRETAIN(decl);
    func->kind = SYM_FUNCTION;
    return true;
}

bool declareVariable(Sema* self, OrbitAST* decl) {
    assert(self && "null semantic checker error");
    assert(decl && "null function decl error");
    assert(decl->kind == ORBIT_AST_DECL_VAR && "invalid variable type declaration");
    
    Scope* scope = currentScope(self);
    OCStringID name = decl->varDecl.name;
    
    // For a function declaration to be valid, its symbol must be either not present in the table,
    // or it its parameter list must be different (overload)
    Symbol* existing = orbit_rcMapGetP(&scope->symbols, name);
    
    if(existing) {
        errorAlreadyDeclared(self, decl, existing->decl);
        return false;
    }
    
    Symbol* var = ORCINIT(ORBIT_ALLOC(Symbol), &symbolDeinit);
    var->decl = ORCRETAIN(decl);
    var->kind = SYM_VARIABLE;
    var->next = NULL;
    orbit_rcMapInsertP(&scope->symbols, name, var);
    return true;
}
