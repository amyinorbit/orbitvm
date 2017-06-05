//
//  orbit/runtime/vm.h
//  Orbit - Runtime
//
//  Created by Amy Parent on 2017-01-03.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_vm_h
#define orbit_vm_h

#include <stdint.h>
#include <orbit/orbit.h>
#include <orbit/utils/assert.h>
#include <orbit/runtime/rtutils.h>
#include <orbit/runtime/value.h>

// We use the X-Macro to define the opcode enum
#define OPCODE(code, idx, stack) CODE_##code,
typedef enum {
#include <orbit/runtime/opcodes.h>
} VMCode;
#undef OPCODE

#define ORBIT_FIRST_GC (32 * 1024)

#define ORBIT_GCSTACK_SIZE 16
struct _OrbitVM {
    VMTask*     task;
    GCObject*   gcHead;
    uint64_t    allocated;
    uint64_t    nextGC;
    
    GCMap*      dispatchTable;
    GCMap*      classes;
    GCMap*      modules;
    
    GCObject*   gcStack[ORBIT_GCSTACK_SIZE];
    uint64_t    gcStackSize;
};

static inline void orbit_gcRetain(OrbitVM* vm, GCObject* object) {
    OASSERT(vm->gcStackSize < ORBIT_GCSTACK_SIZE-1, "stack overflow");
    vm->gcStack[vm->gcStackSize++] = object;
}

static inline void orbit_gcRelease(OrbitVM* vm) {
    OASSERT(vm->gcStackSize > 0, "stack underflow");
    vm->gcStackSize--;
}

void orbit_vmLoadModule(OrbitVM* vm, const char* module);

#endif /* orbit_vm_h */
