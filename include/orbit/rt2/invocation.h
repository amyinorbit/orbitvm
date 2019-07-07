//===--------------------------------------------------------------------------------------------===
// invocation.h - Function calling and FFI API for the Orbit Runtime 2.0
// This source is part of Orbit
//
// Created on 2019-07-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_invocation_h
#define orbit_invocation_h
#include <orbit/common.h>
#include <orbit/rt2/garbage.h>
#include <orbit/rt2/value.h>

void orbit_functionWrite(OrbitGC* gc, OrbitFunction* func, uint8_t code, int32_t line);
void orbit_taskEnsureStack(OrbitTask* self, size_t addedSize);

OrbitFrame* orbit_taskPushFrame(OrbitGC* gc, OrbitTask* self, OrbitFunction* function);
void orbit_taskPopFrame(OrbitGC* gc, OrbitTask* self);

#endif