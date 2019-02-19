//===--------------------------------------------------------------------------------------------===
// orbit/ast/context.h - Contexts are used to track all objects required during compilation
// This source is part of Orbit - AST
//
// Created on 2018-05-17 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_ast_context_h
#define orbit_ast_context_h
#include <orbit/ast/ast.h>
#include <orbit/ast/diag.h>
#include <orbit/csupport/source.h>

typedef struct _OrbitASTContext OrbitASTContext;

struct _OrbitASTContext {
    OrbitSource         source;
    OrbitDiagManager    diagnostics;
    OrbitAST*           root;
};

void orbit_astContextInit(OrbitASTContext* context);
void orbit_astContextDeinit(OrbitASTContext* context);

#endif /* orbit_ast_context_h */
