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
#include <orbit/compiler.h>
#include "sema_private.h"

void pushScope(Sema* self, OCScope* scope);
void popScope(Sema* self);

const OCSymbol* lookupSymbol(Sema* self, OCStringID name);
bool declareFunction(Sema* self, OrbitAST* decl);
bool declareVariable(Sema* self, OrbitAST* decl);
