//===--------------------------------------------------------------------------------------------===
// orbit/ast/type.h - Orbit's compile-type type system API
// This source is part of Orbit - AST
//
// Created on 2018-05-15 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_ast_type_h
#define orbit_ast_type_h
#include <stdbool.h>
#include <orbit/ast/ast.h>
#include <orbit/csupport/string.h>

bool orbit_astTypeIsPrimitive(const OrbitAST* a);
bool orbit_astTypeEqualsPrimitive(const OrbitAST* a, ASTKind b);
bool orbit_astTypeEquals(const OrbitAST* a, const OrbitAST* b);
OrbitAST* orbit_astTypeCopy(const OrbitAST* src);
void orbit_astTypeString(OCStringBuffer* buffer, OrbitAST* ast);

#endif /* orbit_ast_type_h */
