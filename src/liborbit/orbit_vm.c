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

void orbit_gcRun(OrbitVM* vm) {
    // Reset allocation size so we can count as we go
    vm->allocated = 0;
    
    for(uint32_t i = 0; i < vm->sp; ++i) {
        orbit_gcMark(vm, vm->stack[i]);
    }
    
    // TODO: mark functions in dispatch table
    
// basic Mark-sweep algorithm from 
// http://journal.stuffwithstuff.com/2013/12/08/babys-first-garbage-collector/
    GCObject** obj = &vm->gcHead;
    while(*obj) {
        if(!(*obj)->mark) {
            GCObject* garbage = *obj;
            *obj = garbage->next;
            orbit_gcDeallocate(vm, garbage);
        } else {
            (*obj)->mark = false;
            obj = &(*obj)->next;
        }
    }
}

static inline void orbit_markClass(OrbitVM* vm, GCClass* class) {
    vm->allocated += sizeof(GCClass);
}

static inline void orbit_markString(OrbitVM* vm, GCString* string) {
    string->base.mark = true;
    vm->allocated += sizeof(GCString) + string->length + 1;
}

static inline void orbit_markInstance(OrbitVM* vm, GCInstance* instance) {
    instance->base.mark = true;
    // mark objects pointed to by the fields of the instance.
    for(uint16_t i = 0; i < instance->base.class->fieldCount; ++i) {
        orbit_gcMark(vm, instance->fields[i]);
    }
    // mark the class .
    orbit_gcMarkObject(vm, (GCObject*)instance->base.class);
    
    vm->allocated += sizeof(GCInstance)
                    + instance->base.class->fieldCount * sizeof(GCValue);
}

static inline void orbit_markFunction(OrbitVM* vm, VMFunction* function) {
    vm->allocated += sizeof(VMFunction);
    if(function->type == FN_NATIVE) {
        vm->allocated += function->native.byteCodeLength
                         + (function->native.constantCount * sizeof(GCValue));
    }
}

static inline void orbit_markContext(OrbitVM* vm, VMContext* context) {
    vm->allocated += sizeof(VMContext) + context->globalCount * sizeof(GCValue);
    vm->allocated += context->dispatchTable.capacity * sizeof(VMFunction);
    
    // Mark all the globals
    for(uint16_t i = 0; i < context->globalCount; ++i) {
        orbit_gcMark(vm, context->globals[i]);
    }
    
    // Mark registered classes
    for(uint16_t i = 0; i < context->classCount; ++i) {
        orbit_gcMarkObject(vm, (GCObject*)context->classes[i]);
    }
    
    // Mark functions from the context's dispatch table
    for(uint32_t i = 0; i < context->dispatchTable.capacity; ++i) {
        if(context->dispatchTable.data[i]) {
            orbit_gcMarkObject(vm, (GCObject*)context->dispatchTable.data[i]);
        }
    }
}

void orbit_gcMarkObject(OrbitVM* vm, GCObject* obj) {
    if(obj == NULL) return;
    if(obj->mark) return;
    
    obj->mark = true;
    
    switch(obj->type) {
    case OBJ_CLASS:
        orbit_markClass(vm, (GCClass*)obj);
        break;
    case OBJ_INSTANCE:
        orbit_markInstance(vm, (GCInstance*)obj);
        break;
    case OBJ_STRING:
        orbit_markString(vm, (GCString*)obj);
        break;
    case OBJ_FUNCTION:
        orbit_markFunction(vm, (VMFunction*)obj);
        break;
    case OBJ_CONTEXT:
        orbit_markContext(vm, (VMContext*)obj);
        break;
    }
}

void orbit_gcMark(OrbitVM* vm, GCValue value) {
    if(!IS_OBJECT(value)) return;
    orbit_gcMarkObject(vm, AS_OBJECT(value));
}
