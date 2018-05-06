//===--------------------------------------------------------------------------------------------===
// orbit/runtime/gc.c
// This source is part of Orbit - Runtime
//
// Created on 2017-01-12 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/utils/assert.h>
#include <orbit/runtime/gc.h>
#include <orbit/runtime/vm.h>

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
    orbit_gcMarkObject(vm, (OrbitGCObject*)vm->task);
    orbit_gcMarkObject(vm, (OrbitGCObject*)vm->dispatchTable);
    orbit_gcMarkObject(vm, (OrbitGCObject*)vm->classes);
    orbit_gcMarkObject(vm, (OrbitGCObject*)vm->modules);
    
    // mark the retained objects
    for(uint8_t i = 0; i < vm->gcStackSize; ++i) {
        orbit_gcMarkObject(vm, vm->gcStack[i]);
    }
    
    GCDBG("gc run: sweeping");
    
// basic Mark-sweep algorithm from 
// http://journal.stuffwithstuff.com/2013/12/08/babys-first-garbage-collector/
    OrbitGCObject** obj = &vm->gcHead;
    while(*obj) {
        if(!(*obj)->mark) {
            OrbitGCObject* garbage = *obj;
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

static inline void orbit_markClass(OrbitVM* vm, OrbitGCClass* class) {
    vm->allocated += sizeof(OrbitGCClass);
    orbit_gcMarkObject(vm, (OrbitGCObject*)class->name);
    orbit_gcMarkObject(vm, (OrbitGCObject*)class->methods);
}

static inline void orbit_markString(OrbitVM* vm, OrbitGCString* string) {
    vm->allocated += sizeof(OrbitGCString) + string->length + 1;
}

static inline void orbit_markInstance(OrbitVM* vm, OrbitGCInstance* instance) {
    // mark objects pointed to by the fields of the instance.
    for(uint16_t i = 0; i < instance->base.class->fieldCount; ++i) {
        orbit_gcMark(vm, instance->fields[i]);
    }
    // mark the class .
    orbit_gcMarkObject(vm, (OrbitGCObject*)instance->base.class);
    
    vm->allocated += sizeof(OrbitGCInstance)
                    + instance->base.class->fieldCount * sizeof(OrbitValue);
}

static inline void orbit_markMap(OrbitVM* vm, OrbitGCMap* map) {
    vm->allocated += sizeof(OrbitGCMap);
    vm->allocated += sizeof(OrbitGCMapEntry) * map->capacity;
    
    for(uint32_t i = 0; i < map->capacity; ++i) {
        if(IS_NIL(map->data[i].key)) continue;
        orbit_gcMark(vm, map->data[i].key);
        orbit_gcMark(vm, map->data[i].value);
    }
}

static inline void orbit_markArray(OrbitVM* vm, OrbitGCArray* array) {
    vm->allocated += sizeof(OrbitGCArray);
    vm->allocated += sizeof(OrbitValue) * array->capacity;
    
    for(uint32_t i = 0; i < array->size; ++i) {
        orbit_gcMark(vm, array->data[i]);
    }
}

static inline void orbit_markFunction(OrbitVM* vm, VMFunction* function) {
    vm->allocated += sizeof(VMFunction);
    orbit_gcMarkObject(vm, (OrbitGCObject*)function->module);
    if(function->type == ORBIT_FK_NATIVE) {
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
    vm->allocated += sizeof(OrbitValue) * task->stackCapacity;
    
    // mark the stack
    for(OrbitValue* val = task->stack; val < task->sp; val++) {
        orbit_gcMark(vm, *val);
    }
    
    // mark the call frames
    for(uint32_t i = 0; i < task->frameCount; ++i) {
        orbit_gcMarkObject(vm, (OrbitGCObject*)task->frames[i].function);
    }
}

void orbit_gcMarkObject(OrbitVM* vm, OrbitGCObject* obj) {
    if(obj == NULL) return;
    if(obj->mark) return;
    
    obj->mark = true;
    
    switch(obj->type) {
    case ORBIT_OBJK_CLASS:
        orbit_markClass(vm, (OrbitGCClass*)obj);
        break;
    case ORBIT_OBJK_INSTANCE:
        orbit_markInstance(vm, (OrbitGCInstance*)obj);
        break;
    case ORBIT_OBJK_STRING:
        orbit_markString(vm, (OrbitGCString*)obj);
        break;
    case ORBIT_OBJK_MAP:
        orbit_markMap(vm, (OrbitGCMap*)obj);
        break;
    case ORBIT_OBJK_ARRAY:
        orbit_markArray(vm, (OrbitGCArray*)obj);
        break;
    case ORBIT_OBJK_FUNCTION:
        orbit_markFunction(vm, (VMFunction*)obj);
        break;
    case ORBIT_OBJK_MODULE:
        orbit_markModule(vm, (VMModule*)obj);
        break;
    case ORBIT_OBJK_TASK:
        orbit_markTask(vm, (VMTask*)obj);
        break;
    }
}

void orbit_gcMark(OrbitVM* vm, OrbitValue value) {
    if(!IS_OBJECT(value)) return;
    orbit_gcMarkObject(vm, AS_OBJECT(value));
}
