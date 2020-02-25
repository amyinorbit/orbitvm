//===--------------------------------------------------------------------------------------------===
// orbit/rt2/opcodes.h
// This source is part of Orbit - Runtime
//
// Created on 2016-11-13 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
//  Opcodes used by the Orbit Runtime 2.0. This include is meant to be used as an
//  X-macro (see https://en.wikipedia.org/wiki/X_Macro ). The order of
//  declaration of the opcodes defines how the main interpreter loop's jump
//  table is laid out, and should be changed only if necessary.
//
//  The first parameter is the opcode's mnemonic, the second the number of
//  bytes following the opcode in the bytecode stream that should be read as
//  argument.
//
#ifndef OPCODE
#define OPCODE(_, __, ___)
#endif

OPCODE(return, 0, 0)
OPCODE(return_val, 0, -1)
OPCODE(return_repl, 0, -1)
OPCODE(print, 0, 0)

OPCODE(const, 1, 1)
OPCODE(true, 0, 1)
OPCODE(false, 0, 1)

OPCODE(load_local, 1, 1)
OPCODE(store_local, 1, -1)

OPCODE(load_global, 1, 1)
OPCODE(store_global, 1, -1)

OPCODE(i2f, 0, 0)
OPCODE(f2i, 0, 0)

OPCODE(iadd, 0, -1)
OPCODE(isub, 0, -1)
OPCODE(imul, 0, -1)
OPCODE(idiv, 0, -1)

OPCODE(fadd, 0, -1)
OPCODE(fsub, 0, -1)
OPCODE(fmul, 0, -1)
OPCODE(fdiv, 0, -1)

OPCODE(ieq, 0, -1)
OPCODE(ilt, 0, -1)
OPCODE(igt, 0, -1)
OPCODE(ilteq, 0, -1)
OPCODE(igteq, 0, -1)

OPCODE(feq, 0, -1)
OPCODE(flt, 0, -1)
OPCODE(fgt, 0, -1)
OPCODE(flteq, 0, -1)
OPCODE(fgteq, 0, -1)

OPCODE(jump, 2, 0)
OPCODE(rjump, 2, 0)
OPCODE(jump_if, 2, -1)

OPCODE(call, 0, 0)
OPCODE(call_sym, 0, 0)
//
// OPCODE(retain, 0, 0)
// OPCODE(release, 0, -1)
