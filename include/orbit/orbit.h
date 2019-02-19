//===--------------------------------------------------------------------------------------------===
// orbit.h - Main interface for the Orbit Runtime libraries
// This source is part of Orbit
//
// Created on 2016-12-09 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_h
#define orbit_h

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef enum _VMResult  VMResult;
typedef struct _OrbitVM OrbitVM;

enum _VMResult {
    ORBIT_SUCCESS,
    ORBIT_RUNTIME_ERROR
};

OrbitVM* orbit_vmNew();

void orbit_vmDealloc(OrbitVM* vm);

bool orbit_vmInvoke(OrbitVM* vm, const char* module, const char* entry);

#endif /* orbit_h */
