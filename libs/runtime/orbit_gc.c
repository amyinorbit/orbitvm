//
//  orbit_gc.c
//  OrbitVM
//
//  Created by Amy Parent on 2017-01-12.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <orbit/utils/orbit_assert.h>
#include <orbit/runtime/orbit_gc.h>
#include <orbit/runtime/orbit_vm.h>

#ifdef DEBUG_GC
#define GCDBG(fmt, ...) DBG(fmt, ##__VA_ARGS__)
#else
#define GCDBG(fmt, ...)
#endif

void orbit_gcRun(OrbitVM* vm) {
    // Reset allocation size so we can count as we go
    GCDBG("gc run: kick (%llu)", vm->allocated);
    GCDBG("gc run: marking objects");
    
    vm->allocated = 0;
    
    // mark everything used by the current execution context
    orbit_gcMarkObject(vm, (GCObject*)vm->task);
    orbit_gcMarkObject(vm, (GCObject*)vm->dispatchTable);
    orbit_gcMarkObject(vm, (GCObject*)vm->classes);
    orbit_gcMarkObject(vm, (GCObject*)vm->modules);
    
    // mark the retained objects
    for(uint8_t i = 0; i < vm->gcStackSize; ++i) {
        orbit_gcMarkObject(vm, vm->gcStack[i]);
    }
    
    GCDBG("gc run: sweeping");
    
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
    
    GCDBG("gc run: done (%llu)", vm->allocated);
    vm->nextGC = vm->allocated * 2;
}

static inline void orbit_markClass(OrbitVM* vm, GCClass* class) {
    vm->allocated += sizeof(GCClass);
    orbit_gcMarkObject(vm, (GCObject*)class->name);
    orbit_gcMarkObject(vm, (GCObject*)class->methods);
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
    orbit_gcMarkObject(vm, (GCObject*)function->module);
    if(function->type == FN_NATIVE) {
        vm->allocated += function->native.byteCodeLength;
    }
}

static inline void orbit_markModule(OrbitVM* vm, VMModule* module) {
    vm->allocated += sizeof(VMModule)
                   + (module->globalCount * sizeof(VMGlobal));
    
    for(uint16_t i = 0; i < module->globalCount; ++i) {
        orbit_gcMark(vm, module->globals[i].name);
        orbit_gcMark(vm, module->globals[i].global);
    }
    
    for(uint16_t i = 0; i < module->constantCount; ++i) {
        orbit_gcMark(vm, module->constants[i]);
    }
}

static inline void orbit_markTask(OrbitVM* vm, VMTask* task) {
    vm->allocated += sizeof(VMTask);
    vm->allocated += sizeof(VMCallFrame) * task->frameCapacity;
    vm->allocated += sizeof(GCValue) * task->stackCapacity;
    
    // mark the stack
    for(GCValue* val = task->stack; val < task->sp; val++) {
        orbit_gcMark(vm, *val);
    }
    
    // mark the call frames
    for(uint32_t i = 0; i < task->frameCount; ++i) {
        orbit_gcMarkObject(vm, (GCObject*)task->frames[i].function);
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
    case OBJ_MAP:
        orbit_markMap(vm, (GCMap*)obj);
        break;
    case OBJ_ARRAY:
        orbit_markArray(vm, (GCArray*)obj);
        break;
    case OBJ_FUNCTION:
        orbit_markFunction(vm, (VMFunction*)obj);
        break;
    case OBJ_MODULE:
        orbit_markModule(vm, (VMModule*)obj);
        break;
    case OBJ_TASK:
        orbit_markTask(vm, (VMTask*)obj);
        break;
    }
}

void orbit_gcMark(OrbitVM* vm, GCValue value) {
    if(!IS_OBJECT(value)) return;
    orbit_gcMarkObject(vm, AS_OBJECT(value));
}
