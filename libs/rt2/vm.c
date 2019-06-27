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

static inline void push(OrbitVM* vm, OrbitValue value) {
    *(vm->sp) = value;
    vm->sp += 1;
}

static inline OrbitValue pop(OrbitVM* vm) {
    vm->sp -= 1;
    return *(vm->sp);
}

static inline OrbitValue peek(OrbitVM* vm, int distance) {
    return vm->sp[-1 - distance];
}

static inline void debugValue(OrbitValue value) {
    switch(value.kind) {
    case ORBIT_VK_INT: fprintf(stderr, "%d\n", AS_INT(value)); break;
    case ORBIT_VK_FLOAT: fprintf(stderr, "%f\n", AS_FLOAT(value)); break;
    }
}

OrbitResult orbit_run(OrbitVM* vm, OrbitChunk* chunk) {
    assert(vm && "null vm error");
    assert(chunk && "null chunk error");
    vm->chunk = chunk;
    vm->ip = chunk->code;
    
#define READ_BYTE() (*vm->ip++)
#define READ_CONSTANT() (vm->chunk->constants.data[READ_BYTE()])
    
#define BINARY(T, U, op) \
    do {\
        int32_t b = T(pop(vm)); \
        int32_t a = T(pop(vm)); \
        push(vm, U(a op b)); \
    } while(false)
    
    for(;;) {
        
#ifdef ORBIT_DEBUG_TRACE
        orbit_debugInstruction(vm->chunk, vm->ip - vm->chunk->code);
#endif
        
        uint8_t instruction;
        switch(instruction = READ_BYTE()) {
        case OP_RETURN:
            return ORBIT_OK;
            
        case OP_CONST:
            push(vm, READ_CONSTANT());
            break;
            
        case OP_PRINT:
            debugValue(peek(vm, 0));
            break;
            
        case OP_IADD: BINARY(AS_INT, MAKE_INT, +); break;
        case OP_ISUB: BINARY(AS_INT, MAKE_INT, -); break;
        case OP_IMUL: BINARY(AS_INT, MAKE_INT, *); break;
        case OP_IDIV: BINARY(AS_INT, MAKE_INT, /); break;
        
        case OP_FADD: BINARY(AS_FLOAT, MAKE_FLOAT, +); break;
        case OP_FSUB: BINARY(AS_FLOAT, MAKE_FLOAT, -); break;
        case OP_FMUL: BINARY(AS_FLOAT, MAKE_FLOAT, *); break;
        case OP_FDIV: BINARY(AS_FLOAT, MAKE_FLOAT, /); break;
            
        default:
            return ORBIT_RUNTIME_ERROR;
        }
    }
    
#undef READ_BYTE
#undef READ_CONSTANT
}
