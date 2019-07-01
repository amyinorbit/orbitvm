//===--------------------------------------------------------------------------------------------===
// helpers.c - Bookkeeping sema functions to declutter the typechecking sources
// This source is part of Orbit
//
// Created on 2019-07-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/sema/typecheck.h>
#include <orbit/utils/memory.h>

typedef struct {
    ORCObject base;
    
    OrbitAST* decl;
    struct Symbol* next;
} Symbol;

void initScope(OCScope* self, OCScope* parent);
void deinitScope(OCScope* self);
OCScope* pushScope(OCSema* self);
void popScope(OCSema* self);
