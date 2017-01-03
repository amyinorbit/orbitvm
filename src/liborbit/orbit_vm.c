//
//  orbit_vm.c
//  OrbitVM
//
//  Created by Cesar Parent on 2017-01-03.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#include "orbit_vm.h"
#include "orbit_utils.h"


void orbit_vmInit(OrbitVM* vm) {
    OASSERT(vm != NULL, "Null instance error");
    vm->sp = 0;
    vm->ip = NULL;
    vm->gcHead = NULL;
    vm->allocated = 0;
    
    orbit_vtableInit(&vm->dispatchTable);
}

void orbit_gc(OrbitVM* vm) {
    for(uint32_t i = 0; i < vm->sp; ++i) {
        orbit_gcMark(vm, vm->stack[i]);
    }
    // TODO: sweep step;
}

static void orbit_markString(OrbitVM* vm,GCString* string) {
    // TODO: keep track of how many bytes the string uses.
}

static void orbit_markInstance(OrbitVM* vm, GCInstance* instance) {
    for(uint16_t i = 0; i < instance->base.class->fieldCount; ++i) {
        orbit_gcMark(vm, instance->fields[i]);
    }
    // TODO: keep track of how many bytes the object uses.
}

void orbit_mark(OrbitVM* vm, GCValue value) {
    if(!IS_OBJECT(value)) return;
    GCObject* obj = AS_OBJECT(value);
    if(obj == NULL) return;
    if(obj->mark) return;
    
    obj->mark = true;
    
    switch(obj->type) {
    case OBJ_INSTANCE:
        orbit_markInstance(vm, (GCInstance*)obj);
        break;
    case OBJ_STRING:
        orbit_markString(vm, (GCString*)obj);
        break;
    }
}
