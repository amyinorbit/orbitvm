//===--------------------------------------------------------------------------------------------===
// scope.h - Interface used by compiler stages to keep track of scope and variables
// This source is part of Orbit
//
// Created on 2019-07-14 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_scope_h
#define orbit_scope_h
#include <orbit/common.h>
#include <orbit/compiler.h>
#include <orbit/utils/memory.h>
#include <orbit/csupport/string.h>

// The Orbit Compiler is modular - we generate an AST (strictly, a context?). To avoid having to
// write a symbol table for each of the stages (sema and code generation), we can embed one into
// the AST. This is very probably prone to change in the future.

struct sOCSymbol {
    OCStringID name;
    int index;
    bool isVariable;
    const OrbitAST* decl;
};

struct sOCScope {
    OCScope* parent;
    
    int offset;
    int count;
    int capacity;
    OCSymbol* symbols;
};

void orbitScopeInit(OCScope* scope);
void orbitScopeDeinit(OCScope* scope);

const OCSymbol* orbitSymbolLookup(const OCScope* scope, OCStringID name);
const OCSymbol* orbitSymbolDeclare(OCScope* scope, OCStringID name, const OrbitAST* decl);
const OCSymbol* orbitFunctionLookup(const OCScope* scope, OCStringID name, const OrbitAST* args);
const OCSymbol* orbitFunctionDeclare(OCScope* scope, OCStringID name, const OrbitAST* decl);

#endif