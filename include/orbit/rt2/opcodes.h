//===--------------------------------------------------------------------------------------------===
// opcodes.h - Orbit's Opcode enumerator
// This source is part of Orbit
//
// Created on 2019-06-27 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_opcode_enum_h
#define orbit_opcode_enum_h

#define OPCODE(code, _, __) code,

typedef enum {
#include <orbit/rt2/opcodes.inl>
} OrbitCode;

#undef OPCODE

#endif