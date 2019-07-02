//===--------------------------------------------------------------------------------------------===
// errors.c - Semantics checker error wrappers.
// This source is part of Orbit
//
// Created on 2019-07-02 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "errors.h"
#include <orbit/ast/diag.h>

static inline OrbitDiagManager* diag(Sema* self) {
    return &self->context->diagnostics;
}

void errorAssign(Sema* self, OrbitAST* expr) {
    OrbitSRange rng = expr->sourceRange;
    OrbitSLoc loc = expr->binaryExpr.operator.sourceLoc;
    OrbitDiagID id = orbit_diagError(
        diag(self), loc,
        "cannot assign '$0' with a value of type '$1'", 2,
        ORBIT_DIAG_TYPE(expr->binaryExpr.lhs->type),
        ORBIT_DIAG_TYPE(expr->binaryExpr.rhs->type)
    );
    orbit_diagAddSourceRange(id, rng);
}

void errorNameLookup(Sema* self, OrbitAST* expr) {
    OrbitSRange rng = expr->sourceRange;
    OrbitDiagID id = orbit_diagError(
        diag(self), rng.start,
        "no values named '$0' found", 1,
        ORBIT_DIAG_STRING(expr->nameExpr.name)
    );
    orbit_diagAddSourceRange(id, rng);
}

void errorNoType(Sema* self, OrbitAST* var) {
    OrbitSRange rng = var->sourceRange;
    OrbitDiagID id = orbit_diagError(
        diag(self), rng.start,
        "The type of '$0' cannot be deduced, add a type annotation or assign the variable", 1,
        ORBIT_DIAG_STRING(var->varDecl.name)
    );
    orbit_diagAddSourceRange(id, rng);
}

static void previousDeclNote(Sema* self, OrbitAST* prev) {
    OrbitSRange rng = prev->sourceRange;
    OrbitDiagID id = orbit_diagInfo(
        diag(self), rng.start, "the previous declaration of '$0' is here", 1,
        ORBIT_DIAG_STRING(prev->varDecl.name)
    );
    orbit_diagAddSourceRange(id, rng);
}

void errorAlreadyDeclared(Sema* self, OrbitAST* decl, OrbitAST* prev) {
    OrbitSRange rng = decl->sourceRange;
    OrbitDiagID id = orbit_diagError(
        diag(self), rng.start,
        "the name '$0' is already declared in this context", 1,
        ORBIT_DIAG_STRING(decl->varDecl.name)
    );
    orbit_diagAddSourceRange(id, rng);
    if(prev) previousDeclNote(self, prev);
}

void errorBinary(Sema* self, OrbitAST* expr) {
    OrbitSRange rng = expr->sourceRange;
    OrbitSLoc loc = expr->binaryExpr.operator.sourceLoc;
    OrbitDiagID id = orbit_diagError(
        diag(self), loc,
        "invalid binary expression operands: $0 $1 $2 is not declared", 3,
        ORBIT_DIAG_TYPE(expr->binaryExpr.lhs->type),
        ORBIT_DIAG_CSTRING(orbit_tokenString(expr->binaryExpr.operator.kind)),
        ORBIT_DIAG_TYPE(expr->binaryExpr.rhs->type)
    );
    orbit_diagAddSourceRange(id, rng);
}

void warnUnimplemented(Sema* self, OrbitAST* node) {
    OrbitSRange rng = node->sourceRange;
    OrbitDiagID id = orbit_diagError(
        diag(self), rng.start, "language feature not implemented", 0
    );
    orbit_diagAddSourceRange(id, rng);
}
