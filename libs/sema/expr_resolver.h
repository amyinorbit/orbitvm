//===--------------------------------------------------------------------------------------------===
// expr_resolver.h - The expression resolver finds the result of unary & binary expressions
// This source is part of Orbit
//
// Created on 2019-07-02 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef expr_resolver_h
#define expr_resolver_h

#include "sema_private.h"

void declareDefaultOperators(Sema* self);
void declareOperator(Sema* self, OperatorSemData op);

const Conversion* findCast(const OrbitAST* from, const OrbitAST* to);
const OrbitAST* resolveBinaryExpr(Sema* self, OrbitAST* expr);
const OrbitAST* unaryExprResult(Sema* self, OrbitTokenKind op, OrbitAST* rhs);

#endif