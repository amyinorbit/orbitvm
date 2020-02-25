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
#include <orbit/rt2/garbage.h>
#include <orbit/rt2/buffer.h>
#include <orbit/rt2/map.h>

typedef void (*OrbitDestructor)(void*);

typedef enum {
    ORBIT_OBJ_STRING,
    ORBIT_OBJ_MODULE,
    ORBIT_OBJ_FUNCTION,
    ORBIT_OBJ_TASK,
} OrbitObjectKind;

struct sOrbitObject {
    OrbitObjectKind kind;
    OrbitObject* next;
    bool mark:1;
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

typedef struct {
    OrbitString* name;
    OrbitValue value;
} OrbitGlobal;

DECLARE_BUFFER(Global, OrbitGlobal);

struct sOrbitModule {
    OrbitObject base;
    OrbitString* name;
    OrbitGlobalArray globals;
    OrbitGlobalArray functions;
};

struct sOrbitFunction {
    OrbitObject base;

    OrbitModule* module;

    uint8_t arity;
    uint8_t locals;
    uint16_t requiredStack;

    OrbitByteArray code;
    OrbitIntArray lines;
    OrbitValueArray constants;
};

struct sOrbitFrame {
    OrbitFunction* function;
    OrbitValue* base;
    OrbitValue* stack;
    uint8_t* ip;
};

DECLARE_BUFFER(Frame, OrbitFrame);

struct sOrbitTask {
    OrbitObject base;
    uint8_t* ip;

    size_t stackCapacity;
    OrbitValue* stack;
    OrbitValue* stackTop;

    OrbitFrameArray frames;
};

static inline bool ORBIT_IS_STRING(OrbitValue value) {
    return ORBIT_IS_REF(value) && ORBIT_AS_REF(value)->kind == ORBIT_OBJ_STRING;
}

static inline bool ORBIT_IS_FUNCTION(OrbitValue value) {
    return ORBIT_IS_REF(value) && ORBIT_AS_REF(value)->kind == ORBIT_OBJ_FUNCTION;
}

OrbitObject* orbitObjectNew(OrbitGC* gc, OrbitObjectKind kind, size_t size);

OrbitString* orbitStringCopy(OrbitGC* gc, const char* data, int32_t count);
OrbitString* orbitStringNew(OrbitGC* gc, int32_t count);

OrbitModule* orbitModuleNew(OrbitGC* gc);
OrbitFunction* orbitFunctionNew(OrbitGC* gc);
// TODO: replace function with Module?
OrbitTask* orbitTaskNew(OrbitGC* gc, OrbitFunction* function);

void orbitObjectMark(OrbitGC* gc, OrbitObject* self);
void orbitObjectFree(OrbitGC* gc, OrbitObject* self);

#endif