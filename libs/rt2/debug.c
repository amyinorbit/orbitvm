//===--------------------------------------------------------------------------------------------===
// debug.c - Implementation of the runtime debugging functions
// This source is part of Orbit
//
// Created on 2019-06-27 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/rt2/debug.h>
#include <orbit/rt2/opcodes.h>
#include <stdio.h>

typedef struct {
    uint8_t length;
    int8_t effect;
} OpcodeData;

#define OPCODE(inst, length, effect) [ OP_##inst ] = {length, effect},
static const OpcodeData opcodeData[] = {
#include <orbit/rt2/opcodes.inl>
};
#undef OPCODE

#define OPCODE(name, _, __) [ OP_##name ] = #name ,
static const char* opcodeNames[] = {
#include <orbit/rt2/opcodes.inl>
};
#undef OPCODE

void orbit_debugChunk(const OrbitChunk* chunk, const char* name) {
    fprintf(stderr, "** chunk: %s **\n", name);
    // int oldLine = -1;
    for(int offset = 0; offset < chunk->count;) {
        fprintf(stderr, "[%4d] ", offset);
        // int line = chunk->lines[offset];
        // if(line != oldLine)
        //     fprintf(stderr, "%4d ", line);
        // else
        //     fprintf(stderr, "   | ");
        // oldLine = line;
        offset = orbit_debugInstruction(chunk, offset);
    }
}

int orbit_debugInstruction(const OrbitChunk* chunk, int offset) {
    uint8_t code = chunk->code[offset];
    OpcodeData data = opcodeData[code];
    switch(data.length) {
        case 0:
            fprintf(stderr, "%-10s\n", opcodeNames[code]);
            break;
        case 1:
            fprintf(stderr, "%-10s val%3d\n", opcodeNames[code], chunk->code[offset+1]);
            break;
        default: break;
    }
    
    return offset + data.length + 1;
}

