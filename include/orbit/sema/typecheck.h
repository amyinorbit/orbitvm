//===--------------------------------------------------------------------------------------------===
// orbit/sema/typecheck.h
// This source is part of Orbit - Sema
//
// Created on 2017-10-10 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_sema_type_h
#define orbit_sema_type_h

#include <orbit/common.h>
#include <orbit/utils/platforms.h>
#include <orbit/ast/context.h>

#include <orbit/csupport/string.h>
#include <orbit/csupport/rcarray.h>
#include <orbit/csupport/rcmap.h>

typedef struct sOCSema OCSema;
typedef struct sOCScope OCScope;

struct sOCScope {
    OCScope* parent;
    ORCMap types;
    ORCMap symbols;
};

#define ORBIT_SCOPE_MAX 128

struct sOCSema {
    OrbitASTContext* context;
    OCScope global;
    
    OCScope* current;
    OCScope stack[ORBIT_SCOPE_MAX];
};

void orbit_semaInit(OCSema* sema);
void orbit_semaDeinit(OCSema* sema);

void orbit_semaCheck(OCSema* self, OrbitASTContext* context);

#endif /* orbit_sema_type_h */
