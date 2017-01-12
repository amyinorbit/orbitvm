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
//  The first parameter is the opcode's mnemonic, the second the number of
//  bytes following the opcode in the bytecode stream that should be read as
//  argument.
//
#ifndef OPCODE
#define OPCODE(_, _)
#endif

OPCODE(halt, 0)         /// Halts the virtual machine.
OPCODE(const_nil, 0)    /// 
OPCODE(const_true, 0)   /// 
OPCODE(const_false, 0)  /// 
OPCODE(load_const, 1)   /// Pushes a constant from the pool onto the stack.
OPCODE(load_local, 1)   /// Pushes a local variable onto the stack.
OPCODE(load_field, 1)   /// Pushes an object's field onto the stack.
OPCODE(load_global, 1)  /// Pushes a global variable onto the stack.
OPCODE(store_local, 1)  /// Stores the top of the stack in a local variable.
OPCODE(store_field, 1)  /// Stores the top of the stack in an object's field.
OPCODE(store_global, 1) /// Stores the top of the stack into a global variable.
    
OPCODE(add, 0)
OPCODE(sub, 0)
OPCODE(mult, 0)
OPCODE(div, 0)
OPCODE(mod, 0)

OPCODE(and, 1)          ///
OPCODE(or, 1)           ///

/**/

OPCODE(jump_if, 1)      ///
OPCODE(jump, 1)         ///
OPCODE(rjump_if, 1)     ///
OPCODE(rjump, 1)        ///
OPCODE(pop, 0)          ///
OPCODE(swap, 0)         ///

/*
 * Invocation codes - dynamic run-time dispatch means the argument is an index
 * in the constant table for the method signature's string, then looked up
 * in the VM's dispatch table
 */

OPCODE(invoke, 1)       /// Invoke a function
OPCODE(ret_val, 1)      ///
OPCODE(ret, 0)          ///
OPCODE(init, 1)         ///
OPCODE(debug_prt, 1)    ///

#undef OPCODE
