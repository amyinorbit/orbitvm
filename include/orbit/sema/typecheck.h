//===--------------------------------------------------------------------------------------------===
// orbit/sema/typecheck.h
// This source is part of Orbit - Sema
//
// Created on 2017-10-10 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_sema_type_h
#define orbit_sema_type_h

#include <orbit/utils/platforms.h>
#include <orbit/ast/context.h>

void sema_runTypeAnalysis(OrbitASTContext* context);

#endif /* orbit_sema_type_h */
