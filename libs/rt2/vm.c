//===--------------------------------------------------------------------------------------------===
// vm.c - Orbit Runtime/VM 2.0 implementation
// This source is part of Orbit
//
// Created on 2019-06-27 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/rt2/vm.h>
#include <orbit/rt2/debug.h>
#include <orbit/rt2/opcodes.h>
#include <assert.h>
#include <stdio.h>

void orbit_vmInit(OrbitVM* self) {
    assert(self && "null vm error");
    self->sp = self->stack;
}

void orbit_vmDeinit(OrbitVM* self) {
    assert(self && "null vm error");
    
}

static inline void _push(OrbitVM* vm, OrbitValue value) {
    *(vm->sp) = value;
    vm->sp += 1;
}

static inline OrbitValue _pop(OrbitVM* vm) {
    vm->sp -= 1;
    return *(vm->sp);
}

static inline OrbitValue _peek(OrbitVM* vm, int distance) {
    return vm->sp[-1 - distance];
}

OrbitResult orbit_run(OrbitVM* vm, OrbitChunk* chunk) {
    assert(vm && "null vm error");
    assert(chunk && "null chunk error");
    vm->chunk = chunk;
    vm->ip = chunk->code;
    
#define READ_BYTE() (*vm->ip++)
#define READ_CONSTANT() (vm->chunk->constants.data[READ_BYTE()])
    
    
    for(;;) {
        
#ifdef ORBIT_DEBUG_TRACE
        orbit_debugInstruction(vm->chunk, vm->ip - vm->chunk->code);
#endif
        
        uint8_t instruction;
        switch(instruction = READ_BYTE()) {
        case OP_RETURN:
            return ORBIT_OK;
            
        case OP_CONST:
            _push(vm, READ_CONSTANT());
            break;
            
        case OP_PRINT:
            fprintf(stderr, "%d\n", _peek(vm, 0));
            break;
            
        default:
            return ORBIT_RUNTIME_ERROR;
        }
    }
    
#undef READ_BYTE
#undef READ_CONSTANT
}
