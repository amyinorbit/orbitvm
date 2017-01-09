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
#include "orbit_vtable.h"
#include "orbit_value.h"

#define ORBIT_STACK_SIZE 256

typedef struct _OrbitVM {
    uint8_t*    ip;
    uint32_t    sp;
    GCValue     stack[ORBIT_STACK_SIZE];
    
    OrbitVtable dispatchTable;
    
    GCObject*   gcHead;
    size_t      allocated;
} OrbitVM;


void orbit_vmInit(OrbitVM* vm);

static inline void orbit_vmPush(OrbitVM* vm, GCValue value) {
    OASSERT(vm->sp < ORBIT_STACK_SIZE, "stack overflow");
    vm->stack[vm->sp++] = value;
}

static inline GCValue orbit_vmPop(OrbitVM* vm) {
    OASSERT(vm->sp > 0, "stack underflow");
    return vm->stack[--vm->sp];
}

void orbit_gcRun(OrbitVM* vm);

void orbit_gcMarkObject(OrbitVM* vm, GCObject* obj);

void orbit_gcMark(OrbitVM* vm, GCValue value);

#endif /* orbit_vm_h */