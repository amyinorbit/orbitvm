//
//  orbit/parser/recursive_descent.h
//  Orbit - Parser
//
//  Created by Amy Parent on 2018-04-16.
//  Copyright © 2017-2018 Amy Parent. All rights reserved.
//
#ifndef orbit_sema_typeprivate_h
#define orbit_sema_typeprivate_h

#include <orbit/utils/memory.h>
#include <orbit/utils/rcarray.h>

#define ORBIT_SEMA_SCOPESTACK_SIZE 128

typedef struct _OCSema OCSema;
typedef struct _OCScope OCScope;

struct _OCSema {
    ORCArray    uniqueTypes;
    OCScope     stack[ORBIT_SEMA_SCOPESTACK_SIZE];
};

struct _OCScope {
    OCScope*    parent;
    ORCMap      symbolTable;
};

bool sema_typeEquals(AST* a, AST* b);
AST* sema_uniqueTypeExists(OCSema* sema, AST* type);

#endif /* orbit_sema_typeprivate_h */
