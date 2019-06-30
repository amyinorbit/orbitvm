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
#include <unic/unic.h>
#include <assert.h>
#include <string.h>

OrbitObject* orbit_objectNew(OrbitVM* vm, OrbitObjectKind kind, size_t size) {
    assert(vm && "null VM error");
    
    OrbitObject* obj = (OrbitObject*)orbit_allocator(NULL, 0, size);
    obj->kind = kind;
    obj->retainCount = 0;
    obj->mark = false;
    
    obj->next = vm->head;
    vm->head = obj;
    
    return obj;
}

OrbitString* orbit_stringCopy(OrbitVM* vm, const char* data, int32_t count) {
    assert(vm && "null VM error");
    
    OrbitString* self = ALLOC_OBJECT(vm, OrbitString, ORBIT_OBJ_STRING);
    
    self->count = unic_countGraphemes(data, count);
    self->utf8count = count;
    
    memcpy(self->data, data, count);
    self->data[count] = '\0';
    
    return self;
}

OrbitString* orbit_stringNew(OrbitVM* vm, int32_t count) {
    assert(vm && "null VM error");
    
    OrbitString* self = ALLOC_OBJECT(vm, OrbitString, ORBIT_OBJ_STRING);
    
    self->count = 0;
    self->utf8count = count;
    self->data[count] = '\0';
    
    return self;
}

static void freeString(OrbitString* self) {
    DEALLOC_FLEX(self, OrbitString, char, self->utf8count+1);
}

void orbit_objectFree(OrbitObject* self) {
    assert(self && "null object error");
    switch(self->kind) {
        case ORBIT_OBJ_STRING: 
            freeString((OrbitString*)self);
            break;
    }
}

