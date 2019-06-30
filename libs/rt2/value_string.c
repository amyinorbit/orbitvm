//===--------------------------------------------------------------------------------------------===
// value_string.c - Implementation of the Orbit Runtime 2.0 strings
// This source is part of Orbit
//
// Created on 2019-06-29 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/rt2/value_string.h>
#include <orbit/rt2/memory.h>
#include <orbit/rt2/vm.h>
#include <unic/unic.h>
#include <string.h>

OrbitString* orbit_stringNew(OrbitVM* vm, const char* data, int32_t count) {
    OrbitString* self = ALLOC_FLEX(OrbitString, char, count+1);
    orbit_objectInit(&self->base, vm, NULL); // TODO: enable standard classes (String)
    
    self->base.kind = ORBIT_OBJ_STRING;
    self->count = unic_countGraphemes(data, count);
    self->utf8count = count;
    
    memcpy(self->data, data, count);
    self->data[count] = '\0';
    
    return self;
}
