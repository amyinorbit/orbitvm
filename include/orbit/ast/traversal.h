//===--------------------------------------------------------------------------------------------===
// orbit/ast/ast_traversal.h
// This source is part of Orbit - AST
//
// Created on 2017-10-07 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_ast_traversal_h
#define orbit_ast_traversal_h

#include <stdbool.h>
#include <orbit/ast/ast.h>

typedef void (*ASTCallback)(OrbitAST*, void*);

/// Traverses the AST and invokes [callback] when a node which [kind] matches
/// the [filter] mask is found. [userData] can be any arbitrary data required by
/// the callback.
void orbit_astTraverse(OrbitAST* ast, ASTKind filter, void* userData, ASTCallback callback);

#endif /* orbit_ast_traversal_h */
