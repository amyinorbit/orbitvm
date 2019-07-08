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

struct sOrbitModule {
    OrbitObject base;
    OrbitValueBuffer globals;
};

struct sOrbitFunction {
    OrbitObject base;
    
    OrbitModule* module;
    
    uint8_t arity;
    uint8_t locals;
    uint16_t requiredStack;
    
    OrbitByteBuffer code;
    OrbitIntBuffer lines;
    OrbitValueBuffer constants;
};

struct sOrbitFrame {
    OrbitFunction* function;
    OrbitValue* base;
};

DECLARE_BUFFER(Frame, OrbitFrame);

struct sOrbitTask {
    OrbitObject base;
    uint8_t* ip;

    size_t stackCapacity;
    OrbitValue* stack;
    OrbitValue* stackTop;
    
    OrbitFrameBuffer frames;
};

static inline bool ORBIT_IS_STRING(OrbitValue value) {
    return ((value & ORBIT_TAG_VALUE) == 0) && ORBIT_AS_REF(value)->kind == ORBIT_OBJ_STRING;
}

static inline bool ORBIT_IS_FUNCTION(OrbitValue value) {
    return ((value & ORBIT_TAG_VALUE) == 0) && ORBIT_AS_REF(value)->kind == ORBIT_OBJ_FUNCTION;
}

OrbitObject* orbit_objectNew(OrbitGC* gc, OrbitObjectKind kind, size_t size);

OrbitString* orbit_stringCopy(OrbitGC* gc, const char* data, int32_t count);
OrbitString* orbit_stringNew(OrbitGC* gc, int32_t count);

OrbitModule* orbit_moduleNew(OrbitGC* gc);
OrbitFunction* orbit_functionNew(OrbitGC* gc);
// TODO: replace function with Module?
OrbitTask* orbit_taskNew(OrbitGC* gc, OrbitFunction* function); 

void orbit_objectMark(OrbitGC* gc, OrbitObject* self);
void orbit_objectFree(OrbitGC* gc, OrbitObject* self);

#endif