//===--------------------------------------------------------------------------------------------===
// orbit/ast/type.h - Orbit's compile-type type system API
// This source is part of Orbit - AST
//
// Created on 2018-05-15 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbitAST_type_h
#define orbitAST_type_h
#include <stdbool.h>
#include <orbit/ast/ast.h>
#include <orbit/csupport/string.h>

bool orbitASTTypeIsPrimitive(const OrbitAST* a);
bool orbitASTTypeEqualsPrimitive(const OrbitAST* a, ASTKind b);
bool orbitASTTypeEquals(const OrbitAST* a, const OrbitAST* b);
OrbitAST* orbitASTTypeCopy(const OrbitAST* src);
void orbitASTTypeString(OCStringBuffer* buffer, const OrbitAST* ast);

#endif /* orbit_ast_type_h */
