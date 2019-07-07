//===--------------------------------------------------------------------------------------------===
// vm.c - Orbit Runtime/VM 2.0 implementation
// This source is part of Orbit
//
// Created on 2019-06-27 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <orbit/rt2/debug.h>
#include <orbit/rt2/opcodes.h>
#include <orbit/rt2/value_object.h>
#include <orbit/rt2/value_string.h>
#include <orbit/rt2/vm.h>
#include <orbit/rt2/invocation.h>
#include <stdio.h>

void orbit_vmInit(OrbitVM* self) {
    assert(self && "null vm error");
    self->gc.head = NULL;
    self->task = NULL;
}

void orbit_vmDeinit(OrbitVM* self) {
    assert(self && "null vm error");
    orbit_gcRun(&self->gc);
}

static inline void push(OrbitTask* task, OrbitValue value) {
    *(task->stackTop) = value;
    task->stackTop += 1;
}

static inline OrbitValue pop(OrbitTask* task) {
    task->stackTop -= 1;
    return *(task->stackTop);
}

static inline OrbitValue peek(OrbitTask* task, int distance) {
    return task->stackTop[-1 - distance];
}

static inline void debugObject(OrbitObject* object) {
    switch(object->kind) {
    case ORBIT_OBJ_STRING: {
        OrbitString* string = (OrbitString*)object;
        fprintf(stderr, "%.*s\n", string->utf8count, string->data);
    } break;
    case ORBIT_OBJ_FUNCTION: {
        OrbitFunction* func = (OrbitFunction*)object;
        fprintf(stderr, "<func: %p>\n", func);
    } break;
    case ORBIT_OBJ_TASK: {
        OrbitTask* task = (OrbitTask*)object;
        fprintf(stderr, "<task: %p>\n", task);
    } break;
    };
}

static inline void debugValue(OrbitValue value) {
    if(ORBIT_IS_REF(value)) {
        debugObject(ORBIT_AS_REF(value));
        return;
    }
    uint32_t tag = ORBIT_GET_FLAGS(value);
    switch(tag) {
    case ORBIT_TAG_BOOL:
        fprintf(stderr, "%s\n", ORBIT_AS_BOOL(value) ? "true" : "false");
        break;
    case ORBIT_TAG_INT:
        fprintf(stderr, "%d\n", ORBIT_AS_INT(value));
        break;
    case ORBIT_TAG_FLOAT:
        fprintf(stderr, "%f\n", ORBIT_AS_FLOAT(value));
        break;
    default:
        fprintf(stderr, "UNKOWN VALUE TYPE\n");
        break;
    }
}

void orbit_debugTOS(OrbitVM* self) {
    if(!self->task) return;
    if(self->task->stackTop == self->task->stack) return;
    debugValue(peek(self->task, 0));
}

void orbit_debugStack(OrbitVM* self) {
    if(!self->task) return;
    printf("== vm stack ==\n");
    for(OrbitValue* sp = self->task->stack; sp != self->task->stackTop; ++sp) {
        debugValue(*sp);
    }
}

OrbitResult orbit_run(OrbitVM* vm, OrbitFunction* function) {
    assert(vm && "null vm error");
    assert(function && "null chunk error");
    vm->function = function;
    // vm->ip = function->code.data;
    
    vm->task = orbit_taskNew(&vm->gc, function);

#define NEXT() break


#define READ_BYTE() (*vm->task->ip++)
#define READ_CONSTANT() (vm->function->constants.data[READ_BYTE()])

#define BINARY(T, U, op)                                                                           \
    do {                                                                                           \
        int32_t b = T(pop(vm->task));                                                              \
        int32_t a = T(pop(vm->task));                                                              \
        push(vm->task, U(a op b));                                                                 \
    } while(false)

    for(;;) {
#ifdef ORBIT_DEBUG_TRACE
        orbit_debugInstruction(vm->function, vm->ip - vm->function->code.data);
        orbit_debugTOS(vm);
#endif

        uint8_t instruction;
        switch(instruction = READ_BYTE()) {
        case OP_return:
            return ORBIT_OK;

        case OP_const:
            push(vm->task, READ_CONSTANT());
            NEXT();
        case OP_true:
            push(vm->task, ORBIT_VALUE_TRUE);
            NEXT();
        case OP_false:
            push(vm->task, ORBIT_VALUE_FALSE);
            NEXT();

        case OP_print:
            debugValue(peek(vm->task, 0));
            NEXT();

        case OP_i2f:
            push(vm->task, ORBIT_VALUE_FLOAT((float)ORBIT_AS_INT(pop(vm->task))));
            NEXT();
        case OP_f2i:
            push(vm->task, ORBIT_VALUE_INT((int32_t)ORBIT_AS_FLOAT(pop(vm->task))));
            NEXT();

        case OP_iadd:
            BINARY(ORBIT_AS_INT, ORBIT_VALUE_INT, +);
            NEXT();
        case OP_isub:
            BINARY(ORBIT_AS_INT, ORBIT_VALUE_INT, -);
            NEXT();
        case OP_imul:
            BINARY(ORBIT_AS_INT, ORBIT_VALUE_INT, *);
            NEXT();
        case OP_idiv:
            BINARY(ORBIT_AS_INT, ORBIT_VALUE_INT, /);
            NEXT();

        case OP_fadd:
            BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_FLOAT, +);
            NEXT();
        case OP_fsub:
            BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_FLOAT, -);
            NEXT();
        case OP_fmul:
            BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_FLOAT, *);
            NEXT();
        case OP_fdiv:
            BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_FLOAT, /);
            NEXT();

        case OP_ieq:
            BINARY(ORBIT_AS_INT, ORBIT_VALUE_BOOL, ==);
            NEXT();
        case OP_ilt:
            BINARY(ORBIT_AS_INT, ORBIT_VALUE_BOOL, <);
            NEXT();
        case OP_igt:
            BINARY(ORBIT_AS_INT, ORBIT_VALUE_BOOL, >);
            NEXT();
        case OP_ilteq:
            BINARY(ORBIT_AS_INT, ORBIT_VALUE_BOOL, <=);
            NEXT();
        case OP_igteq:
            BINARY(ORBIT_AS_INT, ORBIT_VALUE_BOOL, >=);
            NEXT();

        case OP_feq:
            BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_BOOL, ==);
            NEXT();
        case OP_flt:
            BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_BOOL, <);
            NEXT();
        case OP_fgt:
            BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_BOOL, >);
            NEXT();
        case OP_flteq:
            BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_BOOL, <=);
            NEXT();
        case OP_fgteq:
            BINARY(ORBIT_AS_FLOAT, ORBIT_VALUE_BOOL, >=);
            NEXT();


        default:
            return ORBIT_RUNTIME_ERROR;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef NEXT
}
