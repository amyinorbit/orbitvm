//===--------------------------------------------------------------------------------------------===
// orbit/sema/type.h
// This source is part of Orbit - Sema
//
// Created on 2017-10-10 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_sema_type_h
#define orbit_sema_type_h

#include <orbit/utils/platforms.h>
#include <orbit/ast/ast.h>
//#include <orbit/type/type.h>

AST* sema_extractType(AST* ast);

void sema_extractVariableTypes(AST* ast, void* data);
void sema_extractFunctionTypes(AST* ast, void* data);
void sema_extractLiteralTypes(AST* ast, void* data);

void sema_runTypeAnalysis(AST* ast);

#endif /* orbit_sema_type_h */
