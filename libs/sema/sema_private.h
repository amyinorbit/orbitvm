//===--------------------------------------------------------------------------------------------===
// orbit/parser/recursive_descent.h
// This source is part of Orbit - Parser
//
// Created on 2018-04-16 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_sema_typeprivate_h
#define orbit_sema_typeprivate_h

#include <orbit/ast/ast.h>
#include <orbit/ast/context.h>
#include <orbit/utils/memory.h>
#include <orbit/csupport/string.h>
#include <orbit/csupport/rcarray.h>
#include <orbit/csupport/rcmap.h>

#define ORBIT_SEMA_SCOPESTACK_SIZE 128

typedef struct _OCSema OCSema;
typedef struct _OCScope OCScope;

struct _OCScope {
    OCScope*    parent;
    ORCMap      symbolTable;
};

struct _OCSema {
    //ORCArray    uniqueTypes;
    OrbitASTContext*    context;
    ORCMap              typeTable; // Stores user-defined types (and type aliases in the future)
    uint16_t            stackSize;
    OCScope             stack[ORBIT_SEMA_SCOPESTACK_SIZE];
};

void sema_init(OCSema* sema, OrbitASTContext* context);
void sema_deinit(OCSema* sema);

OCScope* sema_pushScope(OCSema* sema);
void sema_popScope(OCSema* sema);
// OrbitAST* sema_lookSymbol(OCSema* sema, const char* symbol, uint64_t length);
OrbitAST* sema_lookupSymbolP(OCSema* sema, OCStringID symbol);
void sema_declareSymbol(OCSema* sema, OCStringID symbol, OrbitAST* type);
void sema_declareType(OCSema* sema, OCStringID name, OrbitAST* declaration);
OrbitAST* sema_lookupType(OCSema* sema, OCStringID name);

bool sema_typeEquals(OrbitAST* a, OrbitAST* b);
OrbitAST* sema_uniqueTypeExists(OCSema* sema, OrbitAST* type);

OCStringID sema_mangleFuncName(OrbitAST* decl);

#endif /* orbit_sema_typeprivate_h */
