//===--------------------------------------------------------------------------------------------===
// mangle.h - Name mangling API used to standardise Orbit function names after the Sema/AST phase.
// This source is part of Orbit - Sema
//
// Created on 2018-05-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_sema_mangle_h
#define orbit_sema_mangle_h
#include <stdint.h>
#include <stdbool.h>
#include <orbit/ast/ast.h>
#include <orbit/utils/string.h>

void sema_mangleType(AST* type, OCStringBuffer* buffer);
OCStringID sema_mangleFuncName(AST* decl);
char* sema_demangle(const char* mangledName, uint64_t length);

#endif /* orbit_sema_mangle_h */
