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
#include <assert.h>

OrbitString* orbit_stringConcat(OrbitGC* gc, const OrbitString* lhs, const OrbitString* rhs) {
    assert(vm && "null VM pointer");
    assert(lhs && rhs && "null string pointer");
    
    size_t bytes = lhs->utf8count + rhs->utf8count;
    OrbitString* string = orbit_stringNew(gc, bytes);
    
    memcpy(string->data, lhs->data, lhs->utf8count);
    memcpy(string->data + lhs->utf8count, rhs->data, rhs->utf8count);
    
    string->count = unic_countGraphemes(string->data, bytes);
    
    return string;
}
