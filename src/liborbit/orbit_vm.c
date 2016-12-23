//
//  orbit_vm.c
//  OrbitVM
//
//  Created by Cesar Parent on 08/12/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include "orbit_vm.h"
#include "orbit_utils.h"

typedef struct _OrbitVM OrbitVM;


OrbitVM* orbit_createVM() {
    OrbitVM* vm = ALLOC(sizeof(OrbitVM));
    vm->sp = &vm->stack[0];
    vm->collectorHead = NULL;
    return vm;
}

OrbitResult orbit_runByteCode(OrbitVM* vm, OrbitModule* task, const char* main) {
    
    return ORBIT_SUCCESS;
}

void orbit_destroyVM(OrbitVM* vm) {
    DEALLOC(vm);
}
