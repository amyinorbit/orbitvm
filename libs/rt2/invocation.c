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
#include <orbit/rt2/vm.h>
#include <orbit/rt2/value_object.h>

DEFINE_BUFFER(Frame, OrbitFrame);

#define ORBIT_STACK_START 1024

void orbitFunctionWrite(OrbitGC* gc, OrbitFunction* func, uint8_t code, int32_t line) {
    assert(gc && "can't manipulate function without a garbage context");
    assert(func && "can't add code to null function");
    orbitByteArrayWrite(gc, &func->code, code);
    orbitIntArrayWrite(gc, &func->lines, line);
}

void orbitTaskEnsureStack(OrbitTask* self, size_t addedSize) {
    assert(self && "null task error");
    size_t required = (self->stackTop - self->stack) + addedSize;
    if(required <= self->stackCapacity) return;

    size_t oldCapacity = self->stackCapacity;
    while(self->stackCapacity < required)
        self->stackCapacity = ORBIT_GROW_CAPACITY(self->stackCapacity);

    OrbitValue* oldStack = self->stack;
    self->stack = ORBIT_REALLOC_ARRAY(self->stack, OrbitValue, oldCapacity, self->stackCapacity);

    // We need to move the pointers
    if(self->stack == oldStack) return;
    ptrdiff_t offset = self->stack - oldStack;

    self->stackTop += offset;
    for(int i = 0; i < self->frames.count; ++i) {
        self->frames.data[i].base += offset;
    }
}

//It might be worth hoisting that into the interpreter's main loop for speed?
//@@PROFILE
OrbitFrame* orbitTaskPushFrame(OrbitGC* gc, OrbitTask* self, OrbitFunction* function) {
    assert(self && "null task error");
    assert(function && "null function error");


    orbitTaskEnsureStack(self, self->stackCapacity + function->requiredStack);

    OrbitFrame frame;
    frame.function = function;
    frame.base = self->stackTop - function->arity;
    frame.stack = self->stackTop + function->locals;
    orbitFrameArrayWrite(gc, &self->frames, frame);

    self->ip = function->code.data;
    self->stackTop = frame.stack;

    return &self->frames.data[self->frames.count-1];
}

// TODO: this will *have* to be moved to the run loop to handle returning values
void orbitTaskPopFrame(OrbitGC* gc, OrbitTask* self) {
    assert(self && "null task error");
    assert(self->frames.count > 0 && "call stack underflow");
    OrbitFrame* popped = &self->frames.data[--self->frames.count];

    self->stackTop = popped->base;
}
