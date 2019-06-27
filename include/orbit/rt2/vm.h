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

#include <assert.h>
#include <stdint.h>
#include <orbit/orbit.h>
#include <orbit/runtime/rtutils.h>
#include <orbit/runtime/value.h>

// We use the X-Macro to define the opcode enum
#define OPCODE(code, idx, stack) CODE_##code,
typedef enum {
#include <orbit/runtime/opcodes.h>
} VMCode;
#undef OPCODE

#define ORBIT_FIRST_GC (32 * 1024)
#define ORBIT_GCSTACK_SIZE 32

struct _OrbitVM {
    OrbitVMTask*    task;
    OrbitGCObject*  gcHead;
    uint64_t        allocated;
    uint64_t        nextGC;
    
    OrbitGCMap*     dispatchTable;
    OrbitGCMap*     classes;
    OrbitGCMap*     modules;
    
    OrbitGCObject*  gcStack[ORBIT_GCSTACK_SIZE];
    uint64_t        gcStackSize;
};

static inline void orbit_gcRetain(OrbitVM* vm, OrbitGCObject* object) {
    assert(vm->gcStackSize < ORBIT_GCSTACK_SIZE-1 && "stack overflow");
    vm->gcStack[vm->gcStackSize++] = object;
}

static inline void orbit_gcRelease(OrbitVM* vm) {
    assert(vm->gcStackSize > 0 && "stack underflow");
    vm->gcStackSize--;
}

void orbit_vmLoadModule(OrbitVM* vm, const char* module);

#endif /* orbit_vm_h */
