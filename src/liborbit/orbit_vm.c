//
//  orbit_vm.c
//  OrbitVM
//
//  Created by Cesar Parent on 2017-01-03.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#include "orbit_vm.h"
#include "orbit_utils.h"
#include "orbit_gc.h"

void orbit_vmInit(OrbitVM* vm) {
    OASSERT(vm != NULL, "Null instance error");
    vm->task = NULL;
    vm->gcHead = NULL;
    vm->allocated = 0;
    
    vm->gcStackSize = 0;
}
