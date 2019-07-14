//===--------------------------------------------------------------------------------------------===
// value.c - OrbitValue utilities
// This source is part of Orbit
//
// Created on 2019-07-03 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/rt2/value.h>
#include <orbit/rt2/value_object.h>
#include <orbit/rt2/value_string.h>

static bool orbitObjectsEqual(const OrbitObject* lhs, const OrbitObject* rhs) {
    if(lhs == rhs) return true;
    if(lhs->kind != rhs->kind) return false;
    if(lhs->kind == ORBIT_OBJ_STRING)
        return orbitStringEquals((const OrbitString*)lhs, (const OrbitString*)rhs);
    return false;
}

bool orbitValueEquals(OrbitValue lhs, OrbitValue rhs) {
    if(ORBIT_IS_REF(lhs) && ORBIT_IS_REF(rhs))
        return orbitObjectsEqual(ORBIT_AS_REF(lhs), ORBIT_AS_REF(rhs));
    uint64_t T = ORBIT_GET_FLAGS(lhs);
    uint64_t U = ORBIT_GET_FLAGS(rhs);
    if(T != U) return false;
    
    switch(T) {
        //case ORBIT_TAG_BOOL: return ORBIT_AS_BOOL(lhs) == ORBIT_AS_BOOL(rhs);
        case ORBIT_TAG_INT: return ORBIT_AS_INT(lhs) == ORBIT_AS_INT(rhs);
        case ORBIT_TAG_FLOAT: return ORBIT_AS_FLOAT(lhs) == ORBIT_AS_FLOAT(rhs);
        default: break;
    }
    
    return false;
}
