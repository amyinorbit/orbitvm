//
//  orbit_vm.c
//  OrbitVM
//
//  Created by Cesar Parent on 2017-01-03.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#include <stdbool.h>
#include "orbit_vm.h"
#include "orbit_utils.h"
#include "orbit_gc.h"

// We use the X-Macro to define the opcode enum
#define OPCODE(code, _) CODE_##code,
typedef enum {
#include "orbit_opcodes.h"
} VMCode;
#undef OPCODE

void orbit_vmInit(OrbitVM* vm) {
    OASSERT(vm != NULL, "Null instance error");
    vm->task = NULL;
    vm->gcHead = NULL;
    vm->allocated = 0;
    
    vm->gcStackSize = 0;
}

bool orbit_vmRun(OrbitVM* vm, VMTask* task) {
    
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(task != NULL, "Null instance error");
    
    OASSERT(task->frameCount > 0, "task must have an entry point");
    
    vm->task = task;
    VMCode instruction;
    
#define FRAME() (task->frames[task->frameCount-1])
#define PUSH(value) (*(task->sp++) = (value))
#define PEEK() (*(task->sp - 1))
#define POP() (*(--task->sp))
#define READ() (*(FRAME().ip++))
#define REPLACE(val) (*(FRAME().ip - 1) = (val))
    
#define DISPATCH() goto loop
#define CASE_OP(val) case CODE_##val
    
    // Macro to ease building binary math operator opcodes
#define DECL_MATH(operator)                                 \
    do {                                                    \
        GCValue b = POP();                                  \
        GCValue a = POP();                                  \
        PUSH(MAKE_NUM(AS_NUM(a) operator AS_NUM(b)));       \
    } while(false)
    
    // Main loop. Tonnes of opimisations to be done here (obviously)
    loop:
    switch(instruction = (VMCode)READ()) {
        
        CASE_OP(halt):
            return true;
        
        CASE_OP(load_nil):
            PUSH(VAL_NIL);
            DISPATCH();
        
        CASE_OP(load_true):
            PUSH(VAL_TRUE);
            DISPATCH();
            
        CASE_OP(load_false):
            PUSH(VAL_FALSE);
            DISPATCH();
            
        CASE_OP(load_const):
            PUSH(FRAME().function->native.constants[READ()]);
            DISPATCH();
            
            CASE_OP(load_local):
            PUSH(task->frames[task->frameCount-1].stackBase[READ()]);
            DISPATCH();
            
        CASE_OP(load_field):
            {
                GCInstance* obj = AS_INST(POP());
                PUSH(obj->fields[READ()]);
            }
            DISPATCH();
            
        CASE_OP(load_global):
            {
                //TODO: replace globals map with array + symbol list
            }
            DISPATCH();
            
        CASE_OP(store_local):
            FRAME().stackBase[READ()] = POP();
            DISPATCH();
            
        CASE_OP(store_field):
            {
                GCValue val = POP();
                AS_INST(POP())->fields[READ()] = val;
            }
            DISPATCH();
            
        CASE_OP(store_global):
            {
                //TODO: replace globals map with array + symbol list
                DISPATCH();
            }
            
        CASE_OP(add):
            DECL_MATH(+);
            DISPATCH();
            
        CASE_OP(sub):
            DECL_MATH(-);
            DISPATCH();
            
        CASE_OP(mul):
            DECL_MATH(*);
            DISPATCH();
            
        CASE_OP(div):
            DECL_MATH(/);
            DISPATCH();
            
        CASE_OP(and):
            // TODO: implementation
            DISPATCH();
            
        CASE_OP(or):
            // TODO: implementation
            DISPATCH();
            
        CASE_OP(jump_if):
            if(IS_FALSE(PEEK())) DISPATCH();
        CASE_OP(jump):
            FRAME().ip += READ();
            DISPATCH();
            
        CASE_OP(rjump_if):
            if(IS_FALSE(PEEK())) DISPATCH();
        CASE_OP(rjump):
            FRAME().ip -= READ();
            DISPATCH();
            
        CASE_OP(pop):
            POP();
            DISPATCH();
            
        CASE_OP(swap):
            {
                GCValue a = POP();
                GCValue b = POP();
                PUSH(a);
                PUSH(b);
            }
            DISPATCH();
            
        CASE_OP(invoke):
            {
                // TODO: implementation
            }
            DISPATCH();
        
            CASE_OP(ret_val):
            {
                // TODO: implementation
            }
            DISPATCH();
            
        CASE_OP(ret):
            {
                // TODO: implementation
            }
            DISPATCH();
            
        CASE_OP(init):
            {
                uint8_t index = READ();
                GCValue* ref = &(FRAME().function->native.constants[READ()]);
                GCClass* class;
                if(IS_CLASS(*ref)) {
                    class = AS_CLASS(*ref);
                } else {
                    // Lazy resolution, like JVM. If it's a symbolic reference,
                    // then we resolve the class, and replace the entry in the
                    // runtime constant pool with a direct ref to the class.
                    VMFunction* fn = task->frames[task->frameCount-1].function;
                    if(!orbit_gcMapGet(fn->module->classes, *ref, ref)) {
                        return false;
                    }
                    class = AS_CLASS(*ref);
                }
                // Do the construction;
                PUSH(MAKE_OBJECT(orbit_gcInstanceNew(vm, class)));
            }
            DISPATCH();
            
        CASE_OP(debug_prt):
            DISPATCH();
        
        default:
            break;
    }
    
    return true;
}
