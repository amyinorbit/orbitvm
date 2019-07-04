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
#include <orbit/rt2/vm.h>
#include <orbit/rt2/memory.h>
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

// GC: Marking

static inline void markString(OrbitGC* gc, OrbitString* self) {
    gc->allocated += sizeof(OrbitString) + (self->utf8count+1);
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
    }
}

// GC: deallocation

static inline void freeString(OrbitGC* gc, OrbitString* self) {
    ORBIT_DEALLOC_FLEX(self, OrbitString, char, self->utf8count+1);
}

static inline void freeFunction(OrbitGC* gc, OrbitFunction* self) {
    orbit_ByteBufferDeinit(gc, &self->code);
    orbit_IntBufferDeinit(gc, &self->lines);
    orbit_ValueBufferDeinit(gc, &self->constants);
    ORBIT_DEALLOC(self, OrbitFunction);
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
    }
}

