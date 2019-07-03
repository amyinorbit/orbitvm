//===--------------------------------------------------------------------------------------------===
// value_string.h - Orbit Runtime's string object API
// This source is part of Orbit
//
// Created on 2019-06-28 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_value_string_h
#define orbit_value_string_h
#include <orbit/common.h>
#include <orbit/rt2/value_object.h>
#include <string.h>
#include <assert.h>

OrbitString* orbit_stringConcat(OrbitGC* gc, const OrbitString* lhs, const OrbitString* rhs);

static inline bool orbit_stringEquals(const OrbitString* lhs, const OrbitString* rhs) {
    assert(lhs && rhs && "null string pointer");
    return lhs->utf8count == rhs->utf8count
        && lhs->hash == rhs->hash
        && memcmp(lhs->data, rhs->data, lhs->utf8count) == 0;
}

#endif