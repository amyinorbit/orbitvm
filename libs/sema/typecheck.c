//===--------------------------------------------------------------------------------------------===
// typecheck.c - Implementation of the main type checking functions of Orbit
// This source is part of Orbit
//
// Created on 2019-07-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/sema/typecheck.h>
#include <orbit/utils/memory.h>
#include <assert.h>

typedef struct sFnOverload FnOverload;

typedef struct {
    ORCObject base;
    OrbitAST* decl;
    FnOverload* next;
} sFnOverload;

static void initScope(OCScope* self, OCScope* parent) {
    self->parent = parent;
    orbit_rcMapInit(&self->types);
    orbit_rcMapInit(&self->symbols);
}

static void deinitScope(OCScope* self) {
    orbit_rcMapDeinit(&self->types);
    orbit_rcMapDeinit(&self->symbols);
}

static OCScope* pushScope(OCSema* self) {
    OCScope* scope = self->current++;
    OCScope* parent = scope == self->stack ? &self->global : (scope-1);
    initScope(scope, parent);
    return scope;
}

static void popScope(OCSema* self) {
    OCScope* scope = (--self->current);
    deinitScope(scope);
}

void orbit_semaInit(OCSema* self) {
    assert(self && "null semantic checker error");
    initScope(&self->global, NULL);
    self->context = NULL;
    self->current = self->stack;
}

void orbit_semaDeinit(OCSema* self) {
    assert(self && "null semantic checker error");
    while(self->current > self->stack) {
        deinitScope(self->current);
        self->current -= 1;
    }
    deinitScope(&self->global);
}

void orbit_semaCheck(OCSema* self, OrbitASTContext* context) {
    assert(self && "null semantic checker error");
    
}

