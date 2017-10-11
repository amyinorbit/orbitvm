//
//  orbit/ast/ast_traversal.h
//  Orbit - AST
//
//  Created by Amy Parent on 2017-10-07.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_ast_traversal_h
#define orbit_ast_traversal_h

#include <stdbool.h>
#include <orbit/ast/ast.h>

typedef void (*ASTCallback)(AST*, void*);

/// Traverses the AST and invokes [callback] when a node which [kind] matches
/// the [filter] mask is found. [userData] can be any arbitrary data required by
/// the callback.
void ast_traverse(AST* ast, ASTKind filter, void* userData, ASTCallback callback);

#endif /* orbit_ast_traversal_h */
