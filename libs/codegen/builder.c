//===--------------------------------------------------------------------------------------------===
// builder.c - Implementation of the Orbit Bytecode Builder
// This source is part of Orbit
//
// Created on 2019-07-04 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "builder.h"
#include <orbit/ast/builders.h>
#include <orbit/ast/type.h>
#include <orbit/rt2/buffer.h>
#include <orbit/rt2/value_object.h>
#include <orbit/rt2/invocation.h>
#include <assert.h>

DEFINE_BUFFER(Selector, OpSelectData);

static int32_t currentLine(const Builder* builder) {
    return 1; // TODO: implementation (call into source::physicalLoc)
}

void builderInit(Builder* builder, OrbitGC* gc) {
    builder->gc = gc;
    builder->function = NULL;
    builder->context = NULL;
    builder->current = NULL;
    orbit_SelectorBufferInit(&builder->selector);

    // TODO: This is vastly similar to Sema's operator resolver. Might be beneficial to lump those
    // two in a common library?
    // OTOH do we want to have to pull in the runtime every time we want to perform semantic
    // analysis? might not be worth it.
    
#define BINARY_OP(T, op, code) ((OpSelectData){(op), ORBIT_AST_TYPEEXPR_##T, ORBIT_AST_TYPEEXPR_##T, (code)})
    
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(INT, ORBIT_TOK_PLUS, OP_iadd));
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(INT, ORBIT_TOK_MINUS, OP_isub));
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(INT, ORBIT_TOK_STAR, OP_imul));
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(INT, ORBIT_TOK_SLASH, OP_idiv));

    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(INT, ORBIT_TOK_EQEQ, OP_ieq));
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(INT, ORBIT_TOK_LT, OP_ilt));
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(INT, ORBIT_TOK_GT, OP_igt));
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(INT, ORBIT_TOK_LTEQ, OP_ilteq));
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(INT, ORBIT_TOK_GT, OP_igteq));
    
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(FLOAT, ORBIT_TOK_PLUS, OP_fadd));
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(FLOAT, ORBIT_TOK_MINUS, OP_fsub));
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(FLOAT, ORBIT_TOK_STAR, OP_fmul));
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(FLOAT, ORBIT_TOK_SLASH, OP_fdiv));

    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(FLOAT, ORBIT_TOK_EQEQ, OP_feq));
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(FLOAT, ORBIT_TOK_LT, OP_flt));
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(FLOAT, ORBIT_TOK_GT, OP_fgt));
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(FLOAT, ORBIT_TOK_LTEQ, OP_flteq));
    orbit_SelectorBufferWrite(gc, &builder->selector, BINARY_OP(FLOAT, ORBIT_TOK_GT, OP_fgteq));
    
#undef BINARY_OP
}

void builderDeinit(Builder* builder) {
    orbit_SelectorBufferDeinit(builder->gc, &builder->selector);
}

int findConstant(OrbitValueBuffer* constants, OrbitValue value) {
    for(int i = 0; i < constants->count; ++i) {
        if(orbit_valueEquals(constants->data[i], value)) return i;
    }
    return -1;
}

int offset(Builder* builder) {
    return builder->function->code.count;
}

uint8_t emitConstant(Builder* builder, OrbitValue value) {
    OrbitValueBuffer* constants = &builder->function->constants;;
    int existing = findConstant(constants, value);
    if(existing != -1) return (uint8_t)existing;

    assert(constants->count + 1 < UINT8_MAX && "only 256 constants are allowed in a function");
    orbit_ValueBufferWrite(builder->gc, constants, value);
    return (uint8_t)(constants->count-1);
}

int emitInst(Builder* builder, OrbitCode code) {
    orbit_functionWrite(builder->gc, builder->function, code, currentLine(builder));
    return builder->function->code.count - 1;
}

int emitConstInst(Builder* builder, OrbitCode code, OrbitValue value) {
    int offset = emitInst(builder, code);
    uint8_t constantIndex = emitConstant(builder, value);
    orbit_functionWrite(builder->gc, builder->function, constantIndex, currentLine(builder));
    return offset;
}

int emitJump(Builder* builder, OrbitCode code) {
    int line = currentLine(builder);
    emitInst(builder, code);
    int patchOffset = builder->function->code.count;
    orbit_functionWrite(builder->gc, builder->function, 0xff, line);
    orbit_functionWrite(builder->gc, builder->function, 0xff, line);
    return patchOffset;
}

int emitRJump(Builder* builder, OrbitCode code, int target) {
    int line = currentLine(builder);
    int offset = emitInst(builder, code);
    int current = builder->function->code.count + 2; // To account for the two byte jump offset
    uint16_t jump = current - target;
    
    orbit_functionWrite(builder->gc, builder->function, (jump >> 8) & 0x00ff, line);
    orbit_functionWrite(builder->gc, builder->function, jump & 0x00ff, line);
    return offset;
}

void patchJump(Builder* builder, int patch) {
    assert(patch < UINT16_MAX && "jump offset too long");
    int current = builder->function->code.count - 2;
    int offset = current - patch;
    
    uint16_t jump = offset;
    
    builder->function->code.data[patch++] = (jump >> 8) & 0x00ff;
    builder->function->code.data[patch++] = jump & 0x00ff;
}

OrbitCode instSelect(Builder* builder, OrbitTokenKind op, const OrbitAST* lhs, const OrbitAST* rhs) {
    for(int i = 0; i < builder->selector.count; ++i) {
        OpSelectData data = builder->selector.data[i];
        if(op == data.op
           && orbit_astTypeEqualsPrimitive(lhs->type, data.lhsType)
           && orbit_astTypeEqualsPrimitive(rhs->type, data.rhsType)) {
            return data.instruction;
        }
    }
    // TODO: throw error
    return OP_return;
}
