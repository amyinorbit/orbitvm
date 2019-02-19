//===--------------------------------------------------------------------------------------------===
// mangle.h - Name mangling API used to standardise Orbit function names after the Sema/AST phase.
// This source is part of Orbit - Mangling
//
// Created on 2018-05-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_mangling_mangle_h
#define orbit_mangling_mangle_h
#include <stdint.h>
#include <stdbool.h>
#include <orbit/ast/ast.h>
#include <orbit/csupport/string.h>

void orbit_mangleType(OrbitAST* type, OCStringBuffer* buffer);
OCStringID orbit_mangleFuncName(OrbitAST* decl);
OCStringID orbit_demangle(const char* mangledName, uint64_t length);

#endif /* orbit_sema_mangle_h */
