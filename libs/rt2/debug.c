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
#include <orbit/rt2/value_object.h>
#include <stdio.h>

typedef struct {
    uint8_t length;
    int8_t effect;
} OpcodeData;

#define OPCODE(inst, length, effect) [OP_##inst] = {length, effect},
static const OpcodeData opcodeData[] = {
#include <orbit/rt2/opcodes.inl>
};
#undef OPCODE

#define OPCODE(name, _, __) [OP_##name] = #name,
static const char* opcodeNames[] = {
#include <orbit/rt2/opcodes.inl>
};
#undef OPCODE

static inline void debugObject(OrbitObject* object) {
    if(!object) return;
    switch(object->kind) {
    case ORBIT_OBJ_STRING: {
        OrbitString* string = (OrbitString*)object;
        fprintf(stdout, "String: %.*s", string->utf8count, string->data);
    } break;
    case ORBIT_OBJ_FUNCTION: {
        OrbitFunction* func = (OrbitFunction*)object;
        fprintf(stdout, "<func: %p>", func);
    } break;
    case ORBIT_OBJ_TASK: {
        OrbitTask* task = (OrbitTask*)object;
        fprintf(stdout, "<task: %p>", task);
    } break;
    default:
        fprintf(stdout, "<object: %p>", object);
        break;
    };
}

static void debugValue(OrbitValue value) {
    if(ORBIT_IS_REF(value)) {
        debugObject(ORBIT_AS_REF(value));
        return;
    }
    uint32_t tag = ORBIT_GET_FLAGS(value);
    switch(tag) {
    case ORBIT_TAG_BOOL:
        fprintf(stdout, "Bool: %s", ORBIT_AS_BOOL(value) ? "true" : "false");
        break;
    case ORBIT_TAG_INT:
        fprintf(stdout, "Int: %d", ORBIT_AS_INT(value));
        break;
    case ORBIT_TAG_FLOAT:
        fprintf(stdout, "Float: %f", ORBIT_AS_FLOAT(value));
        break;
    default:
        fprintf(stdout, "<unknown type>");
        break;
    }
}

void orbit_debugFunction(const OrbitFunction* fn, const char* name) {
    fprintf(stdout, "** function: %s **\n", name);
    // int oldLine = -1;
    for(int offset = 0; offset < fn->code.count;) {
        fprintf(stdout, "[%4d] ", offset);
        // int line = chunk->lines[offset];
        // if(line != oldLine)
        //     fprintf(stdout, "%4d ", line);
        // else
        //     fprintf(stdout, "   | ");
        // oldLine = line;
        offset = orbit_debugInstruction(fn, offset);
    }
}

int orbit_debugInstruction(const OrbitFunction* fn, int offset) {
    uint8_t code = fn->code.data[offset];
    OpcodeData data = opcodeData[code];
    
    
    
    switch(data.length) {
    case 0:
        fprintf(stdout, "%-16s\n", opcodeNames[code]);
        break;
    case 1:
        if(code == OP_const) {
            fprintf(stdout, "%-16s %02hhx     (", opcodeNames[code], fn->code.data[offset + 1]);
            debugValue(fn->constants.data[fn->code.data[offset + 1]]);
            fprintf(stdout, ")\n");
        } else {
            fprintf(stdout, "%-16s %02hhx\n", opcodeNames[code], fn->code.data[offset + 1]);
        }
        break;
    case 2:
        fprintf(stdout, "%-16s %02hhx %02hhx\n", opcodeNames[code],
        fn->code.data[offset + 1], fn->code.data[offset + 2]);
        break;
    default:
        break;
    }
    return offset + data.length + 1;
}
