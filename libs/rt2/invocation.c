//===--------------------------------------------------------------------------------------------===
// invocation.c - Implements the invocation system for the Orbit Runtime 2.0
// This source is part of Orbit
//
// Created on 2019-07-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <orbit/rt2/buffer.h>
#include <orbit/rt2/invocation.h>
#include <orbit/rt2/memory.h>
#include <orbit/rt2/value_object.h>

#define ORBIT_STACK_START 1024
#define ORBIT_FRAMES_START 64

void orbit_functionWrite(OrbitGC* gc, OrbitFunction* func, uint8_t code, int32_t line) {
    assert(gc && "can't manipulate function without a garbage context");
    assert(func && "can't add code to null function");
    orbit_ByteBufferWrite(gc, &func->code, code);
    orbit_IntBufferWrite(gc, &func->lines, line);
}

void orbit_taskInit(OrbitTask* self) {
    assert(self && "null task error");

    self->stackCapacity = ORBIT_STACK_START;
    self->stack = ORBIT_ALLOC_ARRAY(OrbitValue, self->stackCapacity);
    self->stackTop = self->stack;

    self->framesCapacity = ORBIT_FRAMES_START;
    self->frames = ORBIT_ALLOC_ARRAY(OrbitFrame, self->framesCapacity);
    self->framesTop = self->frames;
}

void orbit_taskDeinit(OrbitTask* self) {
    assert(self && "null task error");
    ORBIT_DEALLOC_ARRAY(self->stack, OrbitValue, self->stackCapacity);
    ORBIT_DEALLOC_ARRAY(self->frames, OrbitFrame, self->framesCapacity);
    self->stack = self->stackTop = NULL;
    self->frames = self->framesTop = NULL;
    self->stackCapacity = self->framesCapacity = 0;
}

static void ensureStack(OrbitTask* self, size_t addedSize) {
    assert(self && "null task error");
    size_t required = (self->stackTop - self->stack) + addedSize;
    if(required < self->stackCapacity) return;

    size_t oldCapacity = self->stackCapacity;
    while(self->stackCapacity < required)
        self->stackCapacity = ORBIT_GROW_CAPACITY(self->stackCapacity);

    OrbitValue* oldStack = self->stack;
    self->stack = ORBIT_REALLOC_ARRAY(self->stack, OrbitValue, oldCapacity, self->stackCapacity);

    // We need to move the pointers
    if(self->stack == oldStack) return;
    ptrdiff_t offset = self->stack - oldStack;

    self->stackTop += offset;
    for(OrbitFrame* frame = self->frames; frame < self->framesTop; ++frame)
        frame->base += offset;
}

static inline void ensureFrames(OrbitTask* self, size_t required) {
    if(required < self->framesCapacity) return;
    size_t oldCapacity = self->framesCapacity;
    while(self->framesCapacity < required)
        self->framesCapacity = ORBIT_GROW_CAPACITY(self->framesCapacity);

    OrbitFrame* oldFrames = self->frames;
    self->frames = ORBIT_REALLOC_ARRAY(self->frames, OrbitFrame, oldCapacity, self->framesCapacity);
    self->framesTop += (self->frames - oldFrames);
}

// It might be worth hoisting that into the interpreter's main loop for speed?
// @@PROFILE
OrbitFrame* orbit_taskPushFrame(OrbitTask* self, OrbitFunction* function) {
    assert(self && "null task error");
    assert(self && "null function error");
    ensureFrames(self, self->framesCapacity + 1);
    ensureStack(self, self->stackCapacity + function->requiredStack);

    OrbitFrame* frame = self->framesTop++;
    frame->function = function;
    frame->base = self->stackTop - function->arity;
    frame->ip = function->code.data;
    return frame;
}

// TODO: this will *have* to be moved to the run loop to handle returning values
void orbit_taskPopFrame(OrbitTask* self) {
    assert(self && "null task error");
    assert(self->framesTop > self->frames && "call stack underflow");
    OrbitFrame* popped = --self->framesTop;

    self->stackTop = popped->base;
}
