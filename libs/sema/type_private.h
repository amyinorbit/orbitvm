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
#include <orbit/utils/string.h>
#include <orbit/utils/memory.h>
#include <orbit/utils/rcarray.h>
#include <orbit/utils/rcmap.h>

#define ORBIT_SEMA_SCOPESTACK_SIZE 128

typedef struct _OCSema OCSema;
typedef struct _OCScope OCScope;

struct _OCScope {
    OCScope*    parent;
    ORCMap      symbolTable;
};

struct _OCSema {
    //ORCArray    uniqueTypes;
    ORCMap      typeTable; // Stores user-defined types (and type aliases in the future)
    uint16_t    stackSize;
    OCScope     stack[ORBIT_SEMA_SCOPESTACK_SIZE];
};

void sema_init(OCSema* sema);
void sema_deinit(OCSema* sema);

OCScope* sema_pushScope(OCSema* sema);
void sema_popScope(OCSema* sema);
// AST* sema_lookSymbol(OCSema* sema, const char* symbol, uint64_t length);
AST* sema_lookupSymbolP(OCSema* sema, OCStringID symbol);
void sema_declareSymbol(OCSema* sema, OCStringID symbol, AST* type);
void sema_declareType(OCSema* sema, OCStringID name, AST* declaration);
AST* sema_lookupType(OCSema* sema, OCStringID name);

bool sema_typeEquals(AST* a, AST* b);
AST* sema_uniqueTypeExists(OCSema* sema, AST* type);

#endif /* orbit_sema_typeprivate_h */
