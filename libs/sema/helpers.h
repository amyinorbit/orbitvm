//===--------------------------------------------------------------------------------------------===
// helpers.c - Bookkeeping sema functions to declutter the typechecking sources
// This source is part of Orbit
//
// Created on 2019-07-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/ast/ast.h>
#include "sema_private.h"

typedef struct Symbol {
    ORCObject base;
    
    enum {SYM_VARIABLE, SYM_FUNCTION, SYM_TYPE} kind;
    
    OrbitAST* decl;
    struct Symbol* next;
} Symbol;

void initScope(Scope* self, Scope* parent);
void deinitScope(Scope* self);

Scope* pushScope(Sema* self);
void popScope(Sema* self);

Symbol* lookupSymbol(Sema* self, OCStringID name);
bool declareFunction(Sema* self, OrbitAST* decl);
bool declareVariable(Sema* self, OrbitAST* decl);
