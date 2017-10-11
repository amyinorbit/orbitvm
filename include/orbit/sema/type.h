//
//  orbit/sema/type.h
//  Orbit - Sema
//
//  Created by Amy Parent on 2017-10-10.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_sema_type_h
#define orbit_sema_type_h

#include <orbit/utils/platforms.h>
#include <orbit/ast/ast.h>
#include <orbit/type/type.h>

Type* sema_extractType(AST* ast);

void sema_extractVariableTypes(AST* ast, void* data);
void sema_extractFunctionTypes(AST* ast, void* data);
void sema_extractLiteralTypes(AST* ast, void* data);

void sema_runTypeAnalysis(AST* ast);

#endif /* orbit_sema_type_h */
