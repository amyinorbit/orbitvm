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
#include <orbit/rt2/chunk.h>
#include <orbit/rt2/garbage.h>

void orbit_codegen(OrbitGC* gc, OrbitChunk* chunk, OrbitASTContext* context);

#endif
