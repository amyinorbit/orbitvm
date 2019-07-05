//===--------------------------------------------------------------------------------------------===
// vm.h - The Orbit Runtime 2.0 virtual machine
// This source is part of Orbit
//
// Created on 2019-06-27 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_vm_h
#define orbit_vm_h
#include <orbit/common.h>
#include <orbit/rt2/chunk.h>
#include <orbit/rt2/garbage.h>
#include <orbit/rt2/value_object.h>

typedef enum {
    ORBIT_OK,
    ORBIT_RUNTIME_ERROR,
    ORBIT_COMPILE_ERROR,
} OrbitResult;

#define ORBIT_STACK_MAX 256

struct sOrbitVM {
    OrbitFunction* function;
    uint8_t* ip;
    OrbitValue* sp;
    OrbitValue stack[ORBIT_STACK_MAX];

    OrbitGC gc;
};

void orbit_vmInit(OrbitVM* self);
void orbit_vmDeinit(OrbitVM* self);
void orbit_debugTOS(OrbitVM* self);
void orbit_debugStack(OrbitVM* self);

// OrbitResult orbit_interpret(const char* source);
OrbitResult orbit_run(OrbitVM* vm, OrbitFunction* chunk);


#endif /* orbit_vm_h */
