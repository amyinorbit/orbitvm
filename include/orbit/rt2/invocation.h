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
#include <orbit/rt2/value_object.h>

void orbitFunctionWrite(OrbitGC* gc, OrbitFunction* func, uint8_t code, int32_t line);
void orbitTaskEnsureStack(OrbitTask* self, size_t addedSize);

OrbitFrame* orbitTaskPushFrame(OrbitGC* gc, OrbitTask* self, OrbitFunction* function);
void orbitTaskPopFrame(OrbitGC* gc, OrbitTask* self);

static inline OrbitFrame* orbitTaskFrame(OrbitTask* self) {
    return self->frames.count ? &self->frames.data[self->frames.count-1] : NULL;
}

#endif