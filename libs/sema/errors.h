//===--------------------------------------------------------------------------------------------===
// errors.h - Helper functions for common semantic errors
// This source is part of Orbit
//
// Created on 2019-07-02 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_semaerrors_h
#define orbit_semaerrors_h

#include <orbit/ast/ast.h>
#include "sema_private.h"

void errorAssign(Sema* self, OrbitAST* expr);
void errorNameLookup(Sema* self, OrbitAST* expr);
void errorNoType(Sema* self, OrbitAST* ast);
void errorAlreadyDeclared(Sema* self, OrbitAST* decl, OrbitAST* prev);
void errorBinary(Sema* self, OrbitAST* expr);

void warnUnimplemented(Sema* self, OrbitAST* ast);
#endif