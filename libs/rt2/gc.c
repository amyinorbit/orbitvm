//===--------------------------------------------------------------------------------------------===
// gc.c - Implements VM garbage collection
// This source is part of Orbit
//
// Created on 2019-06-29 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/rt2/vm.h>
#include <assert.h>

void orbit_vmCollect(OrbitVM* self) {
    assert(self && "null VM error");
    OrbitObject* object = self->head;
    while(object) {
        OrbitObject* next = object->next;
        orbit_objectFree(object);
        object = next;
    }
}
