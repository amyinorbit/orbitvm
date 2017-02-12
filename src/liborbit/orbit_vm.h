//
//  orbit_vm.h
//  OrbitVM
//
//  Created by Cesar Parent on 2017-01-03.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#ifndef orbit_vm_h
#define orbit_vm_h

#include <stdint.h>
#include "orbit_utils.h"
#include "orbit_value.h"

// We use the X-Macro to define the opcode enum
#define OPCODE(code, idx, stack) CODE_##code,
typedef enum {
#include "orbit_opcodes.h"
} VMCode;
#undef OPCODE

#define ORBIT_FIRST_GC (32 * 1024)

#define ORBIT_GCSTACK_SIZE 16
typedef struct _OrbitVM {
    VMTask*     task;
    GCObject*   gcHead;
    uint64_t    allocated;
    uint64_t    nextGC;
    
    GCMap*      dispatchTable;
    GCMap*      classes;
    GCMap*      modules;
    
    GCObject*   gcStack[ORBIT_GCSTACK_SIZE];
    uint64_t    gcStackSize;
} OrbitVM;

static inline void orbit_gcRetain(OrbitVM* vm, GCObject* object) {
    OASSERT(vm->gcStackSize < ORBIT_GCSTACK_SIZE-1, "stack overflow");
    vm->gcStack[vm->gcStackSize++] = object;
}

static inline void orbit_gcRelease(OrbitVM* vm) {
    OASSERT(vm->gcStackSize > 0, "stack underflow");
    vm->gcStackSize--;
}

void orbit_vmInit(OrbitVM* vm);

void orbit_vmDeinit(OrbitVM* vm);

bool orbit_vmInvoke(OrbitVM* vm, const char* entry);

#endif /* orbit_vm_h */
