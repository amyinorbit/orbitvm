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

void errorNoType(Sema* self, OrbitAST* var) {
    OrbitSRange rng = var->sourceRange;
    OrbitDiagID id = orbit_diagError(
        diag(self), rng.start,
        "The type of '$0' cannot be deduced, add a type annotation or assign the variable", 1,
        ORBIT_DIAG_STRING(var->varDecl.name)
    );
    orbit_diagAddSourceRange(id, rng);
}

void errorAlreadyDeclared(Sema* self, OrbitAST* decl, OrbitAST* prev) {
    
}

void errorBinary(Sema* self, OrbitAST* expr) {
    OrbitSRange rng = expr->sourceRange;
    OrbitSLoc loc = expr->binaryExpr.operator.sourceLoc;
    OrbitDiagID id = orbit_diagError(
        diag(self), loc,
        "Invalid binary expression operands: $0 $1 $2 is not declared", 3,
        ORBIT_DIAG_TYPE(expr->binaryExpr.lhs->type),
        ORBIT_DIAG_CSTRING(orbit_tokenString(expr->binaryExpr.operator.kind)),
        ORBIT_DIAG_TYPE(expr->binaryExpr.rhs->type)
    );
    orbit_diagAddSourceRange(id, rng);
}
