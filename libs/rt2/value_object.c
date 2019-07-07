//===--------------------------------------------------------------------------------------------===
// value_object.c - Implementation of Runtime 2.0 garbage-collected objects
// This source is part of Orbit
//
// Created on 2019-06-28 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/rt2/value_object.h>
#include <orbit/rt2/garbage.h>
#include <orbit/rt2/memory.h>
#include <orbit/rt2/invocation.h>// TODO: don't like that too much.
#include <orbit/utils/hashing.h>
#include <unic/unic.h>
#include <assert.h>
#include <string.h>

OrbitObject* orbit_objectNew(OrbitGC* gc, OrbitObjectKind kind, size_t size) {
    assert(gc && "null Garbage Collector error");
    OrbitObject* obj = (OrbitObject*)orbit_gcalloc(gc, NULL, 0, size);
    
    obj->kind = kind;
    obj->retainCount = 0;
    obj->mark = false;
    
    obj->next = gc->head;
    gc->head = obj;
    return obj;
}

OrbitString* orbit_stringCopy(OrbitGC* gc, const char* data, int32_t count) {
    assert(gc && "null Garbage Collector error");
    OrbitString* self = (OrbitString*)orbit_objectNew(gc, ORBIT_OBJ_STRING,
        sizeof(OrbitString) + (count+1)*sizeof(char));
    
    self->count = unic_countGraphemes(data, count);
    self->utf8count = count;
    
    memcpy(self->data, data, count);
    self->data[count] = '\0';
    self->hash = orbit_hashString(self->data, self->utf8count);
    return self;
}

OrbitString* orbit_stringNew(OrbitGC* gc, int32_t count) {
    assert(gc && "null Garbage Collector error");
    OrbitString* self = (OrbitString*)orbit_objectNew(gc, ORBIT_OBJ_STRING,
        sizeof(OrbitString) + (count+1)*sizeof(char));
    self->count = 0;
    self->utf8count = 0;
    self->data[0] = '\0';
    self->hash = orbit_hashString(self->data, self->utf8count);
    
    return self;
}

OrbitFunction* orbit_functionNew(OrbitGC* gc) {
    assert(gc && "null Garbage Collector error");
    OrbitFunction* self = (OrbitFunction*)orbit_objectNew(gc, ORBIT_OBJ_FUNCTION,
                                                          sizeof(OrbitFunction));
    self->arity = 0;
    self->locals = 0;
    self->requiredStack = 0;
    orbit_ByteBufferInit(&self->code);
    orbit_IntBufferInit(&self->lines);
    orbit_ValueBufferInit(&self->constants);
    return self;
}

#define ORBIT_STACK_START 1024

OrbitTask* orbit_taskNew(OrbitGC* gc, OrbitFunction* function) {
    assert(gc && "can't create a task with no garbage collector");
    assert(function && "can't create a task with no function");

    OrbitTask* self = (OrbitTask*)orbit_objectNew(gc, ORBIT_OBJ_TASK, sizeof(OrbitTask));
    self->stackCapacity = ORBIT_STACK_START;
    self->stack = ORBIT_ALLOC_ARRAY(OrbitValue, self->stackCapacity);
    self->stackTop = self->stack;

    orbit_FrameBufferInit(&self->frames);
    orbit_taskPushFrame(gc, self, function);
    return self;
}

// GC: Marking

static inline void markString(OrbitGC* gc, OrbitString* self) {
    // gc->allocated += sizeof(OrbitString) + (self->utf8count+1);
}

static inline void markFunction(OrbitGC* gc, OrbitFunction* self) {
    // This shouldn't actually be required since we are keeping track of deallocation sizes
    // gc->allocated += sizeof(OrbitFunction);
    // gc->allocated += sizeof(OrbitValue) * data;
    
    for(int i = 0; i < self->constants.count; ++i) {
        if(!ORBIT_IS_REF(self->constants.data[i])) continue;
        orbit_objectMark(gc, ORBIT_AS_REF(self->constants.data[i]));
    }
}

static inline void markTask(OrbitGC* gc, OrbitTask* self) {
    for(OrbitValue* val = self->stack; val != self->stackTop; ++val) {
        if(ORBIT_IS_REF(*val))
            orbit_objectMark(gc, ORBIT_AS_REF(*val));
    }
    
    for(int i = 0; i < self->frames.count; ++i) {
        orbit_objectMark(gc, (OrbitObject*)self->frames.data[i].function);
    }
}

void orbit_objectMark(OrbitGC* gc, OrbitObject* self) {
    assert(self && "null object error");
    if(self->mark) return;
    self->mark = true;
    
    switch(self->kind) {
        case ORBIT_OBJ_STRING:
            markString(gc, (OrbitString*)self);
            break;
        case ORBIT_OBJ_FUNCTION:
            markFunction(gc, (OrbitFunction*)self);
            break;
        case ORBIT_OBJ_TASK:
            markTask(gc, (OrbitTask*)self);
            break;
    }
}

// GC: deallocation

static inline void freeString(OrbitGC* gc, OrbitString* self) {
    // ORBIT_DEALLOC_FLEX(self, OrbitString, char, self->utf8count+1);
    orbit_gcalloc(gc, self, sizeof(OrbitString) + self->utf8count + 1, 0);
}

static inline void freeFunction(OrbitGC* gc, OrbitFunction* self) {
    orbit_ByteBufferDeinit(gc, &self->code);
    orbit_IntBufferDeinit(gc, &self->lines);
    orbit_ValueBufferDeinit(gc, &self->constants);
    orbit_gcalloc(gc, self, sizeof(OrbitFunction), 0);
}

static inline void freeTask(OrbitGC* gc, OrbitTask* self) {
    orbit_gcalloc(gc, self->stack, sizeof(OrbitValue) * self->stackCapacity, 0);
    self->stack = self->stackTop = NULL;
    self->stackCapacity = 0;
    
    orbit_FrameBufferDeinit(gc, &self->frames);
    orbit_gcalloc(gc, self, sizeof(OrbitTask), 0);
}

void orbit_objectFree(OrbitGC* gc, OrbitObject* self) {
    assert(self && "null object error");
    switch(self->kind) {
        case ORBIT_OBJ_STRING: 
            freeString(gc, (OrbitString*)self);
            break;
        case ORBIT_OBJ_FUNCTION:
            freeFunction(gc, (OrbitFunction*)self);
            break;
        case ORBIT_OBJ_TASK:
            freeTask(gc, (OrbitTask*)self);
            break;
    }
}

