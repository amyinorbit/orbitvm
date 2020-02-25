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

void orbitVMInit(OrbitVM* self) {
    assert(self && "null vm error");
    orbitGCInit(&self->gc);
    self->task = NULL;
}

void orbitVMDeinit(OrbitVM* self) {
    assert(self && "null vm error");
    orbitGCDeinit(&self->gc);
}

static inline void push(OrbitVM* vm, OrbitValue value) {
    *(vm->task->stackTop++) = value;
}

static inline OrbitValue pop(OrbitVM* vm) {
    return *(--vm->task->stackTop);
}

static inline OrbitValue peek(OrbitVM* vm, int distance) {
    return vm->task->stackTop[-1 - distance];
}

static inline void printObject(OrbitObject* object) {
    if(!object) {
        printf("<null>\n");
        return;
    }

    switch(object->kind) {
    case ORBIT_OBJ_STRING: {
        OrbitString* string = (OrbitString*)object;
        printf("%.*s\n", string->utf8count, string->data);
    } break;
    case ORBIT_OBJ_FUNCTION: {
        OrbitFunction* func = (OrbitFunction*)object;
        printf("<func: %p>\n", func);
    } break;
    case ORBIT_OBJ_TASK: {
        OrbitTask* task = (OrbitTask*)object;
        printf("<task: %p>\n", task);
    } break;
    default:
        printf("<object: %p>\n", object);
        break;
    };
}

static void printValue(OrbitValue value) {
    if(ORBIT_IS_REF(value)) {
        printObject(ORBIT_AS_REF(value));
        return;
    }
    uint32_t tag = ORBIT_GET_FLAGS(value);
    switch(tag) {
    case ORBIT_TAG_TRUE:
    case ORBIT_TAG_FALSE:
        printf("%s\n", ORBIT_AS_BOOL(value) ? "true" : "false");
        break;
    case ORBIT_TAG_INT:
        printf("%d\n", ORBIT_AS_INT(value));
        break;
    case ORBIT_TAG_FLOAT:
        printf("%f\n", ORBIT_AS_FLOAT(value));
        break;
    default:
        printf("UNKOWN VALUE TYPE\n");
        break;
    }
}

void orbitDebugTOS(OrbitVM* self) {
    if(!self->task) return;
    if(self->task->stackTop == self->task->stack) return;
    printf("=> ");
    printValue(peek(self, 0));
}

void orbitDebugStack(OrbitVM* self) {
    if(!self->task) return;
    printf("    --stack--\n");

    for(OrbitValue* sp = self->task->stack; sp != self->task->stackTop; ++sp) {
        printf("    * ");
        printValue(*sp);
    }
    printf("    ---------\n");
}

static inline uint8_t read8(OrbitVM* vm) {
    return *(vm->task->ip++);
}

static inline uint16_t read16(OrbitVM* vm) {
    vm->task->ip += 2;
    return ((uint16_t)vm->task->ip[-2] << 8) | ((uint16_t)vm->task->ip[-1]);
}

static inline OrbitValue readConst(OrbitVM* vm) {
    return vm->function->constants.data[read8(vm)];
}

#define ORBIT_DEBUG_TRACE
OrbitResult orbitRun(OrbitVM* vm, OrbitFunction* function) {
    assert(vm && "null vm error");
    assert(function && "null chunk error");
    vm->function = function;
    vm->task = orbitTaskNew(&vm->gc, function);

    register OrbitTask* task = vm->task;
    register OrbitFrame* frame = task->frames.data;

#define NEXT() break
#define BINARY(type, T, U, op)                                                                                             \
    do {                                                                                        \
        type b = T(pop(vm));                                                                    \
        type a = T(pop(vm));                                                                    \
        push(vm, U(a op b));                                                                    \
    } while(false)

    for(;;) {
#ifdef ORBIT_DEBUG_TRACE
        printf(" size: %d\n", vm->task->stackTop - vm->task->stack);
        orbitDebugInstruction(vm->function, vm->task->ip - vm->function->code.data);
        orbitDebugStack(vm);
        // orbitDebugTOS(vm);
        getchar();
#endif

        uint8_t instruction;
        switch(instruction = read8(vm)) {

        case OP_const:
            push(vm, readConst(vm));
            NEXT();
        case OP_true:
            push(vm, ORBIT_VALUE_TRUE);
            NEXT();
        case OP_false:
            push(vm, ORBIT_VALUE_FALSE);
            NEXT();

        case OP_load_local:
            push(vm, frame->base[read8(vm)]);
            NEXT();

        case OP_store_local:
            frame->base[read8(vm)] = pop(vm);
            NEXT();

        case OP_print:
            printValue(pop(vm));
            NEXT();

        case OP_i2f:
            push(vm, ORBIT_VALUE_FLOAT((float)ORBIT_AS_INT(pop(vm))));
            NEXT();
        case OP_f2i:
            push(vm, ORBIT_VALUE_INT((int32_t)ORBIT_AS_FLOAT(pop(vm))));
            NEXT();

        case OP_iadd:
            BINARY(int32_t, ORBIT_AS_INT, ORBIT_VALUE_INT, +);
            NEXT();
        case OP_isub:
            BINARY(int32_t, ORBIT_AS_INT, ORBIT_VALUE_INT, -);
            NEXT();
        case OP_imul:
            BINARY(int32_t, ORBIT_AS_INT, ORBIT_VALUE_INT, *);
            NEXT();
        case OP_idiv:
            BINARY(int32_t, ORBIT_AS_INT, ORBIT_VALUE_INT, /);
            NEXT();

        case OP_fadd:
            BINARY(float, ORBIT_AS_FLOAT, ORBIT_VALUE_FLOAT, +);
            NEXT();
        case OP_fsub:
            BINARY(float, ORBIT_AS_FLOAT, ORBIT_VALUE_FLOAT, -);
            NEXT();
        case OP_fmul:
            BINARY(float, ORBIT_AS_FLOAT, ORBIT_VALUE_FLOAT, *);
            NEXT();
        case OP_fdiv:
            BINARY(float, ORBIT_AS_FLOAT, ORBIT_VALUE_FLOAT, /);
            NEXT();

        case OP_ieq:
            BINARY(int32_t, ORBIT_AS_INT, ORBIT_VALUE_BOOL, ==);
            NEXT();
        case OP_ilt:
            BINARY(int32_t, ORBIT_AS_INT, ORBIT_VALUE_BOOL, <);
            NEXT();
        case OP_igt:
            BINARY(int32_t, ORBIT_AS_INT, ORBIT_VALUE_BOOL, >);
            NEXT();
        case OP_ilteq:
            BINARY(int32_t, ORBIT_AS_INT, ORBIT_VALUE_BOOL, <=);
            NEXT();
        case OP_igteq:
            BINARY(int32_t, ORBIT_AS_INT, ORBIT_VALUE_BOOL, >=);
            NEXT();

        case OP_feq:
            BINARY(float, ORBIT_AS_FLOAT, ORBIT_VALUE_BOOL, ==);
            NEXT();
        case OP_flt:
            BINARY(float, ORBIT_AS_FLOAT, ORBIT_VALUE_BOOL, <);
            NEXT();
        case OP_fgt:
            BINARY(float, ORBIT_AS_FLOAT, ORBIT_VALUE_BOOL, >);
            NEXT();
        case OP_flteq:
            BINARY(float, ORBIT_AS_FLOAT, ORBIT_VALUE_BOOL, <=);
            NEXT();
        case OP_fgteq:
            BINARY(float, ORBIT_AS_FLOAT, ORBIT_VALUE_BOOL, >=);
            NEXT();

        case OP_jump:
            vm->task->ip += read16(vm);
            NEXT();

        case OP_rjump:
            vm->task->ip -= read16(vm);
            NEXT();

        case OP_jump_if: {
            OrbitValue tos = pop(vm);
            uint16_t offset = read16(vm);
            if(ORBIT_IS_BOOL(tos) && ORBIT_AS_BOOL(tos))
                task->ip += offset;
        } NEXT();

        case OP_call: {
            OrbitValue callee = pop(vm);
            assert(ORBIT_IS_FUNCTION(callee) && "cannot call a non-function object");
            OrbitFunction* fn = (OrbitFunction*)ORBIT_AS_REF(callee);

            orbitFrameArrayWrite(&vm->gc, &task->frames, (OrbitFrame){});
            frame = &task->frames.data[task->frames.count-1];
            frame->base = task->stackTop - fn->arity;
            frame->stack = frame->base + fn->locals;
            frame->function = fn;
            frame->ip = task->ip;

            vm->function = fn;
            task->stackTop = frame->stack;
            task->ip = fn->code.data;
        } NEXT();

        case OP_call_sym: {
            OrbitValue callee = pop(vm);
            assert(ORBIT_IS_STRING(callee) && "cannot resolve a non-string reference");

        } NEXT();

        case OP_return: {
            task->stackTop = frame->base;
            task->ip = frame->ip;
            frame = &task->frames.data[--task->frames.count];
            vm->function = frame->function;
            if(!task->frames.count) return ORBIT_OK;
        } NEXT();

        case OP_return_val: {
            OrbitValue value = pop(vm);
            task->stackTop = frame->base;
            task->ip = frame->ip;
            frame = &task->frames.data[--task->frames.count];
            vm->function = frame->function;
            push(vm, value);
            if(!task->frames.count) return ORBIT_OK;;

        }  NEXT();

        case OP_return_repl:
            if(vm->task->stackTop != orbitTaskFrame(vm->task)->stack) {
                OrbitValue value = pop(vm);
                orbitTaskPopFrame(&vm->gc, vm->task);
                push(vm, value);
            } else {
                orbitTaskPopFrame(&vm->gc, vm->task);
            }
            if(!vm->task->frames.count) return ORBIT_OK;
            NEXT();

        default:
            return ORBIT_RUNTIME_ERROR;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef NEXT
}
