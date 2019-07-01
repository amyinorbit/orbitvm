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
#include <assert.h>
#include "helpers.h"

void initScope(OCScope* self, OCScope* parent) {
    self->parent = parent;
    orbit_rcMapInit(&self->types);
    orbit_rcMapInit(&self->symbols);
}

void deinitScope(OCScope* self) {
    orbit_rcMapDeinit(&self->types);
    orbit_rcMapDeinit(&self->symbols);
}

OCScope* pushScope(OCSema* self) {
    OCScope* scope = self->current++;
    OCScope* parent = scope == self->stack ? &self->global : (scope-1);
    initScope(scope, parent);
    return scope;
}

void popScope(OCSema* self) {
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