//===--------------------------------------------------------------------------------------------===
// codegen.h - Main interface to the Orbit code generation system
// This source is part of Orbit
//
// Created on 2019-07-03 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_codegen_h
#define orbit_codegen_h

#include <orbit/ast/context.h>
#include <orbit/rt2/value.h>
#include <orbit/rt2/garbage.h>

OrbitFunction* orbitCodegen(OrbitGC* gc, OrbitASTContext* context);

#endif
