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


bool orbitASTTypeIsPrimitive(const OrbitAST* T);
bool orbitASTTypeEqualsPrimitive(const OrbitAST* T, ASTKind U);
bool orbitASTTypeEquals(const OrbitAST* T, const OrbitAST* U);
OrbitAST* orbitASTTypeCopy(const OrbitAST* T);
void orbitASTTypeString(OCStringBuffer* buffer, const OrbitAST* T);

// Type analysis and manipulation

bool orbitTypeIsCastable(const OrbitAST* From, const OrbitAST* To);
OrbitAST* orbitTypeCast(OrbitAST* expr, const OrbitAST* T);

bool orbitTypesSameOverload(const OrbitAST* T, const OrbitAST* U);
bool orbitTypeIsCallable(const OrbitAST* T);
bool orbitTypeCanCall(const OrbitAST* T, const OrbitAST* Arg);

#endif /* orbit_ast_type_h */
