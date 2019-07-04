//===--------------------------------------------------------------------------------------------===
// gc.c - Implements VM garbage collection
// This source is part of Orbit
//
// Created on 2019-06-29 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <orbit/rt2/garbage.h>
#include <orbit/rt2/value_object.h>
#include <string.h>

typedef OrbitObject* ObjRef;

void orbit_gcInit(OrbitGC* self) {
    assert(self && "null Garbage Collector error");
    self->rootCount = 0;
    self->allocated = 0;
    self->nextCollection = (1 << 16);
    self->head = NULL;
}

void orbit_gcDeinit(OrbitGC* self) {
    assert(self && "null Garbage Collector error");
    self->rootCount = 0;
    orbit_gcRun(self);
    orbit_gcInit(self);
}

static void markObject(OrbitObject* obj) {
    if(!obj) return;
    if(obj->mark) return;
    obj->mark = true;
    // TODO: dispatch to individual mark function
}

static void markRoots(OrbitGC* self) {
    for(size_t i = 0; i < self->rootCount; ++i)
        markObject(self->roots[i]);
}

void orbit_gcRun(OrbitGC* self) {
    assert(self && "null Garbage Collector error");
    // We only mark objects in the roots stack ourselves. The VM will mark the rest
    markRoots(self);
    assert(self && "null Garbage Collector error");
    // OrbitObject* object = self->head;

    ObjRef* ptr = &self->head;
    while(*ptr) {
        if(!(*ptr)->mark) {
            OrbitObject* garbage = *ptr;
            *ptr = garbage->next;
            orbit_objectFree(self, garbage);
        } else {
            (*ptr)->mark = false;
            ptr = &(*ptr)->next;
        }
    }
}

void orbit_gcPush(OrbitGC* self, OrbitObject* root) {
    assert(self && "null Garbage Collector error");
    assert(root && "can't add a null object as a root");
    assert(self->rootCount < ORBIT_GC_MAXROOTS && "garbage collector roots overflow");
    self->roots[self->rootCount++] = root;
}
void orbit_gcPop(OrbitGC* self) {
    assert(self && "null Garbage Collector error");
    assert(self->rootCount < ORBIT_GC_MAXROOTS && "garbage collector roots overflow");
    self->roots[--self->rootCount] = NULL;
}

void orbit_gcRelease(OrbitGC* self, OrbitObject* ref) {
    assert(self && "null Garbage Collector error");
    assert(ref && "can't remove a null object from the root stack");
    assert(self->rootCount > 0);
    // This is a bit trickier, we need to do a search in the array

    ObjRef* shouldBeRemoved = NULL;
    ObjRef* end = self->roots + self->rootCount;

    for(ObjRef* root = self->roots; root != end; ++root) {
        if(*root != ref) continue;
        shouldBeRemoved = root;
        break;
    }

    size_t distance = end - (shouldBeRemoved + 1);

    assert(shouldBeRemoved && "object is not on the root stack");
    memmove(shouldBeRemoved, shouldBeRemoved + 1, distance * sizeof(ObjRef));
    self->rootCount -= 1;
}
