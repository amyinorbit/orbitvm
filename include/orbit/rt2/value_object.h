//===--------------------------------------------------------------------------------------------===
// value_object.h - The base "class" for reference types in the Orbit Runtime 2.0
// This source is part of Orbit
//
// Created on 2019-06-28 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_value_object_h
#define orbit_value_object_h
#include <orbit/common.h>
#include <orbit/rt2/value.h>

typedef struct sOrbitObject OrbitObject;
typedef struct sOrbitInstance OrbitInstance;
typedef void (*OrbitDestructor)(void*);

typedef struct sOrbitString OrbitString;

#define ORBIT_VALUE_REF(value) ((uint64_t)(value) & ORBIT_MASK_REF)
#define ORBIT_AS_REF(value) (OrbitObject*)((uintptr_t)((value) & ORBIT_MASK_REF))

typedef enum {
    ORBIT_OBJ_STRING,
} OrbitObjectKind;

struct sOrbitObject {
    OrbitObjectKind kind;
    OrbitObject* next;
    uint32_t mark:1;
    uint32_t retainCount:31;
};

struct sOrbitInstance {
    OrbitObject base;
    OrbitValue fields[];
};

struct sOrbitString {
    OrbitObject base;
    int32_t count;
    int32_t utf8count;  // This is different than OrbitString::count, which represents graphemes 
    uint32_t hash;
    char data[];        // We use the flexible array member trick to avoid double-alloc
};

OrbitObject* orbit_objectNew(OrbitVM* vm, OrbitObjectKind kind, size_t size);

OrbitString* orbit_stringCopy(OrbitVM* vm, const char* data, int32_t count);
OrbitString* orbit_stringNew(OrbitVM* vm, int32_t count);

void orbit_objectFree(OrbitObject* self);

#endif