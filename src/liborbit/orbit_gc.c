//
//  orbit_gc.c
//  OrbitVM
//
//  Created by Cesar Parent on 2017-01-12.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#include "orbit_gc.h"
#include "orbit_utils.h"

void orbit_gcRun(OrbitVM* vm) {
    // Reset allocation size so we can count as we go
    vm->allocated = 0;
    
    // mark everything used by the current execution context
    orbit_gcMarkObject(vm, (GCObject*)vm->currentContext);
    
    // mark the retained objects
    for(uint8_t i = 0; i < vm->gcStackSize; ++i) {
        orbit_gcMarkObject(vm, vm->gcStack[i]);
    }
    
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
    vm->allocated += sizeof(char) * (class->name.length+1);
}

static inline void orbit_markString(OrbitVM* vm, GCString* string) {
    vm->allocated += sizeof(GCString) + string->length + 1;
}

static inline void orbit_markInstance(OrbitVM* vm, GCInstance* instance) {
    // mark objects pointed to by the fields of the instance.
    for(uint16_t i = 0; i < instance->base.class->fieldCount; ++i) {
        orbit_gcMark(vm, instance->fields[i]);
    }
    // mark the class .
    orbit_gcMarkObject(vm, (GCObject*)instance->base.class);
    
    vm->allocated += sizeof(GCInstance)
                    + instance->base.class->fieldCount * sizeof(GCValue);
}

static inline void orbit_markMap(OrbitVM* vm, GCMap* map) {
    vm->allocated += sizeof(GCMap);
    vm->allocated += sizeof(GCMapEntry) * map->capacity;
    
    for(uint32_t i = 0; i < map->capacity; ++i) {
        if(IS_NIL(map->data[i].key)) continue;
        orbit_gcMark(vm, map->data[i].key);
        orbit_gcMark(vm, map->data[i].value);
    }
}

static inline void orbit_markArray(OrbitVM* vm, GCArray* array) {
    vm->allocated += sizeof(GCArray);
    vm->allocated += sizeof(GCValue) * array->capacity;
    
    for(uint32_t i = 0; i < array->size; ++i) {
        orbit_gcMark(vm, array->data[i]);
    }
}

static inline void orbit_markFunction(OrbitVM* vm, VMFunction* function) {
    vm->allocated += sizeof(VMFunction);
    if(function->type == FN_NATIVE) {
        vm->allocated += function->native.byteCodeLength
                         + (function->native.constantCount * sizeof(GCValue));
    }
}

static inline void orbit_markContext(OrbitVM* vm, VMContext* context) {
    vm->allocated += sizeof(VMContext);
    
    // Mark local variables from the stack
    for(uint32_t i = 0; i < context->sp; ++i) {
        orbit_gcMark(vm, context->stack[i]);
    }
    
    orbit_gcMarkObject(vm, (GCObject*)context->globals);
    orbit_gcMarkObject(vm, (GCObject*)context->classes);
    orbit_gcMarkObject(vm, (GCObject*)context->dispatchTable);
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
    case OBJ_MAP:
        orbit_markMap(vm, (GCMap*)obj);
        break;
    case OBJ_ARRAY:
        orbit_markArray(vm, (GCArray*)obj);
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
