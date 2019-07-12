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

// void errorConditional(Sema* self, OrbitAST* expr);
void errorNotCallable(Sema* self, const OrbitAST* call);
void errorInvalidCall(Sema* self, const OrbitAST* call);

void errorAssign(Sema* self, const OrbitAST* expr);
void errorNameLookup(Sema* self, const OrbitAST* expr);
void errorNoType(Sema* self, const OrbitAST* ast);
void errorAlreadyDeclared(Sema* self, const OrbitAST* decl, const OrbitAST* prev);
void errorBinary(Sema* self, const OrbitAST* expr);

void errorCondition(Sema* self, const char* type, const OrbitAST* expr);

void warnUnimplemented(Sema* self, const OrbitAST* ast);
#endif