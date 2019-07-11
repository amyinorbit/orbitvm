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
#include <orbit/utils/hashing.h>
#include <unic/unic.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

OrbitString* orbitStringConcat(OrbitGC* gc, const OrbitString* lhs, const OrbitString* rhs) {
    assert(gc && "null Garbage Collector pointer");
    assert(lhs && rhs && "null string pointer");
    
    size_t utf8count = lhs->utf8count + rhs->utf8count;
    OrbitString* string = orbitStringNew(gc, utf8count);
    
    memcpy(string->data, lhs->data, lhs->utf8count);
    memcpy(string->data + lhs->utf8count, rhs->data, rhs->utf8count);
    
    string->utf8count = utf8count;
    string->count = unic_countGraphemes(string->data, utf8count);
    string->hash = orbitHashString(string->data, string->utf8count);
    
    return string;
}
