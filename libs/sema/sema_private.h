//===--------------------------------------------------------------------------------------------===
// sema_private.h - Private types needed by Sema
// This source is part of Orbit
//
// Created on 2019-07-02 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef sema_private_h
#define sema_private_h
#include <orbit/ast/ast.h>
#include <orbit/ast/context.h>
#include <orbit/csupport/tokens.h>
#include <orbit/csupport/rcarray.h>
#include <orbit/csupport/rcmap.h>

#define ORBIT_SCOPE_MAX 128

// OperatorSemData only works for primitive types at the moment. We'll see later how
// we implement operator overloading with complex types
typedef struct {
    enum {OP_UNARY, OP_BINARY} kind;
    ASTKind lhsType;
    ASTKind rhsType;
    OrbitTokenKind operator;
    ASTKind result;
} OperatorSemData;

typedef struct {
    ASTKind from;
    ASTKind to;
    bool isImplicit;
    ASTKind nodeKind;
} Conversion;

typedef struct {
    size_t count;
    size_t capacity;
    OperatorSemData* operators;
} ExprResolver;

typedef struct Scope {
    struct Scope* parent;
    ORCMap types;
    ORCMap symbols;
} Scope;

typedef struct {
    OrbitASTContext* context;
    Scope global;
    
    Scope* current;
    Scope stack[ORBIT_SCOPE_MAX];

    ExprResolver resolver;
} Sema;

void orbit_semaInit(Sema* sema);
void orbit_semaDeinit(Sema* sema);

void resolverInit(ExprResolver* self);
void resolverDeinit(ExprResolver* self);

#endif
