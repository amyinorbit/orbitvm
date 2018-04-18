//===--------------------------------------------------------------------------------------------===
// orbit/runtime/opcodes.h
// This source is part of Orbit - Runtime
//
// Created on 2016-11-13 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
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
#define OPCODE(_, _, _)
#endif

OPCODE(halt, 0, 0)          /// Stops VM
OPCODE(load_nil, 0, 1)      /// [...]       -> [..., nil]
OPCODE(load_true, 0, 1)     /// [...]       -> [..., true]
OPCODE(load_false, 0, 1)    /// [...]       -> [..., false]
OPCODE(load_const, 2, 1)    /// [...]       -> [..., constants[idx16]]
OPCODE(load_local, 1, 1)    /// [...]       -> [..., locals[idx8]]
OPCODE(load_field, 2, 0)    /// [..., obj]  -> [..., obj[idx16]]
OPCODE(load_global, 2, 1)   /// [...]       -> [..., globals[idx16]]
OPCODE(store_local, 1, -1)  /// [..., val]  -> [...], locals[idx8] = val
OPCODE(store_field, 2, -2)  /// [..., val, ojb]  -> [...], obj[idx16] = val
OPCODE(store_global, 2, -1) /// [..., val]  -> [...], globals[idx16] = val
    
OPCODE(add, 0, -1)          /// [..., b, a] -> [..., a+b]
OPCODE(sub, 0, -1)          /// [..., b, a] -> [..., a-b]
OPCODE(mul, 0, -1)          /// [..., b, a] -> [..., a*b]
OPCODE(div, 0, -1)          /// [..., b, a] -> [..., a/b]

OPCODE(test_lt, 0, -1)      /// [..., b, a] -> [..., (a<b)]
OPCODE(test_gt, 0, -1)      /// [..., b, a] -> [..., (a>b)]
OPCODE(test_eq, 0, -1)      /// [..., b, a] -> [..., (a==b)]

OPCODE(and, 2, 0)           /// 
OPCODE(or, 2, 0)            ///

/**/

OPCODE(jump_if, 2, -1)      /// [..., val] -> [..., val], if(val) ip += idx16
OPCODE(jump, 2, 0)          /// [...] -> [...], if(val) ip += idx16
OPCODE(rjump_if, 2, -1)     /// [..., val] -> [..., val], if(val) ip -= idx16
OPCODE(rjump, 2, 0)         /// [...] -> [...], if(val) ip -= idx16
OPCODE(pop, 0, -1)          /// [..., val] -> [...]
OPCODE(swap, 0, 0)          /// [..., a, b] -> [..., b, a]

/*
 * Invocation codes - dynamic run-time dispatch means the argument is an index
 * in the constant table for the method signature's string, then looked up
 * in the VM's dispatch table
 */

#ifdef USE_MSGSEND
OPCODE(msgsend, 2, -1)    /// Potential future opcode - class dispatch
#endif
OPCODE(invoke_sym, 2, -1)   /// [..., ref] -> [...], call(dispatch[ref])
OPCODE(invoke, 2, -1)       /// [..., func] -> [...], call(func)
OPCODE(ret_val, 0, 0)       /// [..., [frame]] -> [..., ret_val]
OPCODE(ret, 0, 0)           /// [..., [frame]] -> [...]
OPCODE(init_sym, 2, 1)      /// [...] -> [..., new(classes[constants[idx16]])]
OPCODE(init, 2, 1)          /// [...] -> [..., new(constants[idx16])]
OPCODE(debug_prt, 0, 0)     /// [...] -> [...]

#undef OPCODE
