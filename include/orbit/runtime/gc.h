//===--------------------------------------------------------------------------------------------===
// orbit/runtime/gc.h
// This source is part of Orbit - Runtime
//
// Created on 2017-01-12 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_runtime_gc_h
#define orbit_runtime_gc_h

#include <orbit/orbit.h>
#include <orbit/runtime/value.h>
#include <orbit/runtime/vm.h>

void orbit_gcRun(OrbitVM* vm);

void orbit_gcMarkObject(OrbitVM* vm, OrbitGCObject* obj);

void orbit_gcMark(OrbitVM* vm, OrbitValue value);

#endif /* orbit_runtime_gc_h */
