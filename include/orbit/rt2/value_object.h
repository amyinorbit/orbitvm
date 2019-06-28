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

typedef struct sOrbitObject OrbitObject;
typedef struct sOrbitClass OrbitClass;
typedef struct sOrbitInstance OrbitInstance;
typedef void (*OrbitDestructor)(void*);

typedef struct {
    ORBIT_OBJ_CLASS,
    ORBIT_OBJ_BYTECODE,
} OrbitObjectKind;

struct sOrbitObject {
    OrbitClass* isa;
    OrbitObject* next;
    size_t refCount;
};

struct sOrbitClass {
    OrbitObject base;
    // 
};

struct sOrbitInstance {
    OrbitObject base;
    OrbitValue fields[];
};

#endif