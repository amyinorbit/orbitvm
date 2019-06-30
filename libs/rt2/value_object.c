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
#include <assert.h>

void orbit_objectInit(OrbitObject* self, OrbitVM* vm, const OrbitClass* isa) {
    assert(self && "null object error");
    assert(vm && "null VM error");
    
    self->isa = isa;
    self->next = vm->head;
    vm->head = self;
    self->retainCount = 0;
    self->mark = false;
}

static void freeString(OrbitString* self) {
    DEALLOC_FLEX(self, OrbitString, char, self->utf8count+1);
}

static void freeClass(OrbitClass* self) {
    DEALLOC(self, OrbitClass);
}

void orbit_objectFree(OrbitObject* self) {
    assert(self && "null object error");
    switch(self->kind) {
        case ORBIT_OBJ_CLASS:
            freeClass((OrbitClass*)self);
            break;
        case ORBIT_OBJ_STRING: 
            freeString((OrbitString*)self);
            break;
    }
}

