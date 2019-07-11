//===--------------------------------------------------------------------------------------------===
// orbit/ast/ast_traversal.h
// This source is part of Orbit - AST
//
// Created on 2017-10-07 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_ast_traversal_h
#define orbit_ast_traversal_h

#include <stdbool.h>
#include <orbit/ast/ast.h>
#include <orbit/ast/context.h>

typedef struct sOrbitASTVisitor OrbitASTVisitor;
typedef bool (*OrbitASTPredicate)(const OrbitASTVisitor*, const OrbitAST*);
typedef void (*OrbitASTCallback)(OrbitASTContext* ctx, OrbitAST*, void*);

struct sOrbitASTVisitor {
    OrbitASTCallback    callback;
    OrbitASTPredicate   predicate;
    ASTKind             filter;
    void*               data;
};

OrbitASTVisitor orbitASTSimpleVisitor(OrbitASTCallback callback, ASTKind filter, void* data);
OrbitASTVisitor orbitASTVisitor(OrbitASTCallback callback, OrbitASTPredicate pred, void* data);

/// Traverses the AST and invokes [callback] when a node which [kind] matches
/// the [filter] mask is found. [userData] can be any arbitrary data required by
/// the callback.
void orbitASTTraverse(OrbitASTContext* ctx, OrbitASTVisitor visitor);

#endif /* orbitAST_traversal_h */
