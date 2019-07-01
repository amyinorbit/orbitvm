//===--------------------------------------------------------------------------------------------===
// invocation.h - Function calling and FFI API for the Orbit Runtime 2.0
// This source is part of Orbit
//
// Created on 2019-07-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_invocation_h
#define orbit_invocation_h
#include <orbit/rt2/value.h>
#include <orbit/rt2/garbage.h>

typedef struct sOrbitTask OrbitTask;
typedef struct sOrbitFrame OrbitFrame;

struct sOrbitTask {
    uint8_t* ip;
    
    size_t stackCapacity;
    OrbitValue* stack;
    OrbitValue* stackTop;
    
    size_t framesCapacity;
    OrbitFrame* frames;
    OrbitFrame* framesTop;
};

struct sOrbitFrame {
    OrbitFunction* function;
    uint8_t* ip;
    OrbitValue* base;
};

void orbit_taskInit(OrbitTask* self);
void orbit_taskDeinit(OrbitTask* self);

OrbitFrame* orbit_taskPushFrame(OrbitTask* self, OrbitFunction* function);
void orbit_taskPopFrame(OrbitTask* self);

#endif