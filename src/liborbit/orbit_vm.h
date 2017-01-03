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
    
    GCValue*    gcHead;
    size_t      allocated;
} OrbitVM;


void orbit_vmInit(OrbitVM* vm);

void orbit_gc(OrbitVM* vm);

void orbit_mark(OrbitVM* vm, GCValue value);

#endif /* orbit_vm_h */