//
//  orbit_opcodes.h
//  OrbitVM
//
//  Created by Cesar Parent on 2016-11-13.
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

OPCODE(halt)            /// Halts the virtual machine.
OPCODE(const_nil)       /// 
OPCODE(const_true)      /// 
OPCODE(const_false)     /// 
OPCODE(load_const)      /// Pushes a constant from the pool onto the stack.
OPCODE(load_local)      /// Pushes a local variable onto the stack.
OPCODE(load_field)      /// Pushes an object's field onto the stack.
OPCODE(load_global)     /// Pushes a global variable onto the stack.
OPCODE(store_local)     /// Stores the top of the stack in a local variable.
OPCODE(store_field)     /// Stores the top of the stack in an object's field.
OPCODE(store_global)    /// Stores the top of the stack into a global variable.

OPCODE(and)             ///
OPCODE(or)              ///

/**/

OPCODE(jump_if)         ///
OPCODE(jump)            ///
OPCODE(rjump_if)        ///
OPCODE(rjump)           ///
OPCODE(pop)             ///
OPCODE(swap)            ///

/*
 * Invocation codes - dynamic run-time dispatch means the argument is an index
 * in the constant table for the method signature's string, then looked up
 * in the VM's dispatch table
 */

OPCODE(invoke)          /// Invoke a function
OPCODE(ret_val)         ///
OPCODE(ret)             ///
OPCODE(init)            ///
OPCODE(debug_prt)       ///

#undef OPCODE
