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
#include <orbit/rt2/value_object.h>
#include <assert.h>
#include <stdio.h>

void orbit_vmInit(OrbitVM* self) {
    assert(self && "null vm error");
    self->sp = self->stack;
    self->head = NULL;
}

void orbit_vmDeinit(OrbitVM* self) {
    assert(self && "null vm error");
    orbit_vmCollect(self);
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

static inline void debugObject(OrbitObject* object) {
    switch(object->kind) {
    case ORBIT_OBJ_CLASS:
        fprintf(stderr, "<orbit class>\n");
        break;
    case ORBIT_OBJ_STRING:
        {
            OrbitString* string = (OrbitString*)object;
            fprintf(stderr, "%.*s\n", string->utf8count, string->data);
        }
        break;
    };
}

static inline void debugValue(OrbitValue value) {
    if(ORBIT_IS_REF(value)) {
        debugObject(ORBIT_AS_REF(value));
        return;
    }
    uint32_t tag = ORBIT_GET_FLAGS(value);
    switch(tag) {
    case ORBIT_TAG_BOOL: fprintf(stderr, "%s\n", ORBIT_AS_BOOL(value) ? "true":"false"); break;
    case ORBIT_TAG_INT: fprintf(stderr, "%d\n", ORBIT_AS_INT(value)); break;
    case ORBIT_TAG_FLOAT: fprintf(stderr, "%f\n", ORBIT_AS_FLOAT(value)); break;
    }
}

void orbit_debugTOS(OrbitVM* self) {
    if(self->sp == self->stack) return;
    debugValue(peek(self, 0));
}

OrbitResult orbit_run(OrbitVM* vm, OrbitChunk* chunk) {
    assert(vm && "null vm error");
    assert(chunk && "null chunk error");
    vm->chunk = chunk;
    vm->ip = chunk->code;
    
#define NEXT() break
    
    
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
        case OP_RETURN: return ORBIT_OK;
            
        case OP_CONST: push(vm, READ_CONSTANT()); NEXT();
        case OP_TRUE: push(vm, ORBIT_VALUE_TRUE); NEXT();
        case OP_FALSE: push(vm, ORBIT_VALUE_FALSE); NEXT();
            
        case OP_PRINT: debugValue(peek(vm, 0)); NEXT();
        
        case OP_I2F: push(vm, ORBIT_VALUE_FLOAT((float)ORBIT_AS_INT(pop(vm))));
        case OP_F2I: push(vm, ORBIT_VALUE_INT((int32_t)ORBIT_AS_FLOAT(pop(vm))));
            
        case OP_IADD: BINARY(ORBIT_AS_INT, ORBIT_VALUE_INT, +); NEXT();
        case OP_ISUB: BINARY(ORBIT_AS_INT, ORBIT_VALUE_INT, -); NEXT();
        case OP_IMUL: BINARY(ORBIT_AS_INT, ORBIT_VALUE_INT, *); NEXT();
        case OP_IDIV: BINARY(ORBIT_AS_INT, ORBIT_VALUE_INT, /); NEXT();
        
        case OP_FADD: BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_FLOAT, +); NEXT();
        case OP_FSUB: BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_FLOAT, -); NEXT();
        case OP_FMUL: BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_FLOAT, *); NEXT();
        case OP_FDIV: BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_FLOAT, /); NEXT();
        
        case OP_IEQ: BINARY(ORBIT_AS_INT, ORBIT_VALUE_BOOL, ==); NEXT();
        case OP_ILT: BINARY(ORBIT_AS_INT, ORBIT_VALUE_BOOL, <); NEXT();
        case OP_IGT: BINARY(ORBIT_AS_INT, ORBIT_VALUE_BOOL, >); NEXT();
        case OP_ILTEQ: BINARY(ORBIT_AS_INT, ORBIT_VALUE_BOOL, <=); NEXT();
        case OP_IGTEQ: BINARY(ORBIT_AS_INT, ORBIT_VALUE_BOOL, >=); NEXT();
        
        case OP_FEQ: BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_BOOL, ==); NEXT();
        case OP_FLT: BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_BOOL, <); NEXT();
        case OP_FGT: BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_BOOL, >); NEXT();
        case OP_FLTEQ: BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_BOOL, <=); NEXT();
        case OP_FGTEQ: BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_BOOL, >=); NEXT();
        
            
        default:
            return ORBIT_RUNTIME_ERROR;
        }
    }
    
#undef READ_BYTE
#undef READ_CONSTANT
#undef NEXT
}
