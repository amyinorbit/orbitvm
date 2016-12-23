//
//  OrbitOpcodes.h
//  OrbitVM
//
//  Created by Cesar Parent on 13/11/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
//  Opcodes used by the Orbit VM. This include is meant to be used as an
//  X-macro (see https://en.wikipedia.org/wiki/X_Macro ). The order of
//  declaration of the opcodes defines how the main interpreter loop's jump
//  table is laid out, and should be changed only if necessary.
//

#ifndef OPCODE
#define OPCODE(_)
#endif

OPCODE(HCF)             /// Halts the virtual machine.
OPCODE(CONST)           /// Pushes a constant from the pool onto the stack.
OPCODE(CONST_NULL)      /// 
OPCODE(CONST_TRUE)      /// 
OPCODE(CONST_FALSE)     /// 
OPCODE(LOAD_LOCAL)      /// Pushes a local variable onto the stack.
OPCODE(LOAD_FIELD)      /// Pushes an object's field onto the stack.
OPCODE(LOAD_GLOBAL)     /// Pushes a global variable onto the stack.
OPCODE(LOAD_ARRAY)      ///
OPCODE(LOAD_DICT)       ///
OPCODE(STORE_LOCAL)     /// Stores the top of the stack in a local variable.
OPCODE(STORE_FIELD)     /// Stores the top of the stack in an object's field.
OPCODE(STORE_GLOBAL)    /// Stores the top of the stack into a global variable.
OPCODE(STORE_ARRAY)     ///
OPCODE(STORE_DICT)      ///

OPCODE(AND)             ///
OPCODE(OR)              ///

/**/

OPCODE(JUMP_IF)         ///
OPCODE(JUMP_IFNOT)      ///
OPCODE(JUMP)            ///
OPCODE(RJUMP_IF)        ///
OPCODE(RJUMP_IFNOT)     ///
OPCODE(RJUMP)           ///
OPCODE(POP)             ///
OPCODE(SWAP)            ///

/*
 * Invocation codes - dynamic run-time dispatch means the argument is an index
 * in the constant table for the method signature's string, then looked up
 * in the VM's dispatch table
 */

OPCODE(INVOKE)          /// Generic invocation, with number of args as param
OPCODE(INVOKE_0)        /// Call function with 0 params.
OPCODE(INVOKE_1)        ///
OPCODE(INVOKE_2)        ///
OPCODE(INVOKE_3)        ///
OPCODE(INVOKE_4)        ///
OPCODE(INVOKE_5)        ///
OPCODE(INVOKE_6)        ///
OPCODE(INVOKE_7)        ///
OPCODE(INVOKE_8)        ///
OPCODE(INVOKE_9)        ///
OPCODE(INVOKE_10)       ///
OPCODE(INVOKE_11)       ///
OPCODE(INVOKE_12)       ///
OPCODE(INVOKE_13)       ///
OPCODE(INVOKE_14)       ///
OPCODE(RETURN_VAL)      ///
OPCODE(RETURN)          ///
OPCODE(INIT)            ///
OPCODE(DBG_PRT)         ///

#undef OPCODE
