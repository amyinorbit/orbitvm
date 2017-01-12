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
    
    // pull stuff in locals so we don't have to follow 10 pointers every
    // two line. This means invoke: and return: will have to update those
    // so that we stay on the same page.
    VMCode instruction;
    
    VMCallFrame* frame = &task->frames[task->frameCount-1];
    VMFunction* fn = frame->function;
    uint8_t* ip = frame->ip;
    GCValue* locals = frame->stackBase;
    
#define FRAME() (task->frames[task->frameCount-1])
#define PUSH(value) (*(task->sp++) = (value))
#define PEEK() (*(task->sp - 1))
#define POP() (*(--task->sp))
    
#define READ8() (*(ip++))
#define READ16() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))
    
#define NEXT() goto loop
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
    switch(instruction = (VMCode)READ8()) {
        
        CASE_OP(halt):
            return true;
        
        CASE_OP(load_nil):
            PUSH(VAL_NIL);
            NEXT();
        
        CASE_OP(load_true):
            PUSH(VAL_TRUE);
            NEXT();
            
        CASE_OP(load_false):
            PUSH(VAL_FALSE);
            NEXT();
            
        CASE_OP(load_const):
            PUSH(fn->native.constants[READ8()]);
            NEXT();
            
        CASE_OP(load_local):
            PUSH(locals[READ8()]);
            NEXT();
            
        CASE_OP(load_field):
            {
                GCInstance* obj = AS_INST(POP());
                PUSH(obj->fields[READ8()]);
            }
            NEXT();
            
        CASE_OP(load_global):
            {
                //TODO: replace globals map with array + symbol list
            }
            NEXT();
            
        CASE_OP(store_local):
            locals[READ8()] = POP();
            NEXT();
            
        CASE_OP(store_field):
            {
                GCValue val = POP();
                AS_INST(POP())->fields[READ8()] = val;
            }
            NEXT();
            
        CASE_OP(store_global):
            {
                //TODO: replace globals map with array + symbol list
                NEXT();
            }
            
        CASE_OP(add):
            DECL_MATH(+);
            NEXT();
            
        CASE_OP(sub):
            DECL_MATH(-);
            NEXT();
            
        CASE_OP(mul):
            DECL_MATH(*);
            NEXT();
            
        CASE_OP(div):
            DECL_MATH(/);
            NEXT();
            
        CASE_OP(and):
            // TODO: implementation
            NEXT();
            
        CASE_OP(or):
            // TODO: implementation
            NEXT();
            
        CASE_OP(jump_if):
            if(IS_FALSE(PEEK())) NEXT();
        CASE_OP(jump):
            {
                uint16_t offset = READ16();
                ip += offset;
                NEXT();
            }
            
        CASE_OP(rjump_if):
            if(IS_FALSE(PEEK())) NEXT();
        CASE_OP(rjump):
            {
                uint16_t offset = READ16();
                ip -= offset;
                NEXT();
            }
            
        CASE_OP(pop):
            POP();
            NEXT();
            
        CASE_OP(swap):
            {
                GCValue a = POP();
                GCValue b = POP();
                PUSH(a);
                PUSH(b);
            }
            NEXT();
            
            
        {
            // invoke family of opcodes. When compiled, all invocations are
            // done through `invoke_sym`, and point to a symbolic reference
            // (string in the function's constant pool).
            //
            // The first time an invocation happens, the symbolic reference is
            // resolved (through the module's symbol table). The opcode is
            // replaced with `invoke` and the constant changed to point to the
            // function object in memory. This avoids the overhead of hashmap
            // lookup with every single invocation, but does not require the
            // whole bytecode to be checked and doctored at load time.
            GCValue callee;
            uint8_t idx;
            
        CASE_OP(invoke_sym):
            
            idx = READ8();
            GCValue symbol = fn->native.constants[idx];
            orbit_gcMapGet(fn->module->dispatchTable, symbol, &callee);
            if(!IS_FUNCTION(callee)) return false;
            
            // replace the opcode in the bytecode stream so that future calls
            // can use the direct reference.
            ip[-2] = CODE_invoke;
            fn->native.constants[idx] = callee;
            
            // Start invocation.
            goto do_invoke;
            
        CASE_OP(invoke):
            // Invoke a function by direct reference: by then, the entry in the
            // run-time constant pool points to a function object rather than
            // a string, and we can just go along.
            callee = fn->native.constants[READ8()];
        do_invoke:
            
            OASSERT(IS_FUNCTION(callee), "invoke must be given a function");
            // First, we need to store the data brought up into locals back
            // into the task's frame stack.
            frame->ip = ip;
        
            switch(AS_FUNCTION(callee)->type) {
            case FN_NATIVE:
                // TODO: ensure we have enough frames in the task.
            
                // Get the pointer to the function object for convenience
                fn = AS_FUNCTION(callee);
                
                // setup a new frame on the task's call stack
                frame = &task->frames[task->frameCount++];
                frame->task = task;
                frame->function = fn;
                frame->ip = fn->native.byteCode;
                
                // The stack base points to the first parameter
                frame->stackBase = task->sp - fn->parameterCount;
                locals = frame->stackBase;
                
                // And now we bring up the new frame's IP into the local.
                // NEXT() will start the new function.
                ip = frame->ip;
                NEXT();
                break;
                
            case FN_FOREIGN:
                // TODO: implement Foreign Function invocation
                fn = AS_FUNCTION(callee);
                fn->foreign(task->sp - fn->parameterCount);
                fn = frame->function;
                NEXT();
                break;
            }
        }
            
        CASE_OP(ret):
            {
                // TODO: implementation
                
            }
            NEXT();
            
        CASE_OP(init):
            {
                GCValue* ref = &fn->native.constants[READ8()];
                GCClass* class;
                if(IS_CLASS(*ref)) {
                    class = AS_CLASS(*ref);
                } else {
                    // Lazy resolution, like JVM. If it's a symbolic reference,
                    // then we resolve the class, and replace the entry in the
                    // runtime constant pool with a direct ref to the class.
                    if(!orbit_gcMapGet(fn->module->classes, *ref, ref)) {
                        return false;
                    }
                    class = AS_CLASS(*ref);
                }
                // Do the construction;
                PUSH(MAKE_OBJECT(orbit_gcInstanceNew(vm, class)));
            }
            NEXT();
            
        CASE_OP(debug_prt):
            NEXT();
        
        default:
            break;
    }
    
    return true;
}
