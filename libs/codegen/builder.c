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
#include <orbit/csupport/string.h>
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

void openFunctionGC(Builder* builder, OrbitFunction* function) {
    assert(builder && "cannot open a codegen function without a builder");
    Function* fn = ORCINIT(ORBIT_ALLOC(Function), NULL);
    fn->parent = builder->function;
    fn->impl = function;
    // TODO: we probably need to start worrying about garbage collection here. This is also where
    // it would be useful to maybe start using reference counting in the VM too, instead of GC?
    fn->localCount = 0;
    fn->maxLocals = 0;
    builder->function = ORCRETAIN(fn);
}

#define GC_FUNC() builder->function->impl


int openScope(Builder* builder) {
    return builder->function->localCount;
}

void dropScope(Builder* builder, int stack) {
    builder->function->localCount = stack;
}

void openFunction(Builder* builder) {
    assert(builder && "cannot open a codegen function without a builder");
    openFunctionGC(builder, orbitFunctionNew(builder->gc));
}

int localVariable(Builder* builder, OCStringID name) {
    assert(builder && "cannot locate a codegen variable without a builder");
    Function* fn = builder->function;
    for(int i = 0; i < fn->localCount; ++i) {
        if(name == fn->locals[i]) return i;
    }
    assert(fn->localCount < UINT8_MAX && "too many locals");
    int idx = fn->localCount++;
    fn->locals[idx] = name;
    if(fn->localCount > fn->maxLocals)
        fn->maxLocals = fn->localCount;
    return idx;
}

void closeFunction(Builder* builder) {
    assert(builder && "cannot close a codegen function without a builder");
    Function* fn = builder->function;
    GC_FUNC()->locals = fn->maxLocals;
    builder->function = fn->parent;
    ORCRELEASE(fn);
}

int findConstant(OrbitValueBuffer* constants, OrbitValue value) {
    for(int i = 0; i < constants->count; ++i) {
        if(orbitValueEquals(constants->data[i], value)) return i;
    }
    return -1;
}

int offset(Builder* builder) {
    return GC_FUNC()->code.count;
}


uint8_t emitConstant(Builder* builder, OrbitValue value) {
    OrbitValueBuffer* constants = &GC_FUNC()->constants;;
    int existing = findConstant(constants, value);
    if(existing != -1) return (uint8_t)existing;

    assert(constants->count + 1 < UINT8_MAX && "only 256 constants are allowed in a function");
    orbit_ValueBufferWrite(builder->gc, constants, value);
    return (uint8_t)(constants->count-1);
}

int emitInst(Builder* builder, OrbitCode code) {
    orbitFunctionWrite(builder->gc, GC_FUNC(), code, currentLine(builder));
    return GC_FUNC()->code.count - 1;
}

int emitConstInst(Builder* builder, OrbitCode code, OrbitValue value) {
    int offset = emitInst(builder, code);
    uint8_t constantIndex = emitConstant(builder, value);
    orbitFunctionWrite(builder->gc, GC_FUNC(), constantIndex, currentLine(builder));
    return offset;
}

int emitLocalInst(Builder* builder, OrbitCode code, OCStringID name) {
    int offset = emitInst(builder, code);
    uint8_t localIndex = (uint8_t)localVariable(builder, name);
    orbitFunctionWrite(builder->gc, GC_FUNC(), localIndex, currentLine(builder));
    return offset;
}

int emitJump(Builder* builder, OrbitCode code) {
    int line = currentLine(builder);
    emitInst(builder, code);
    int patchOffset = GC_FUNC()->code.count;
    orbitFunctionWrite(builder->gc, GC_FUNC(), 0xff, line);
    orbitFunctionWrite(builder->gc, GC_FUNC(), 0xff, line);
    return patchOffset;
}

int emitRJump(Builder* builder, OrbitCode code, int target) {
    int line = currentLine(builder);
    int offset = emitInst(builder, code);
    int current = GC_FUNC()->code.count + 2; // To account for the two byte jump offset
    uint16_t jump = current - target;
    
    orbitFunctionWrite(builder->gc, GC_FUNC(), (jump >> 8) & 0x00ff, line);
    orbitFunctionWrite(builder->gc, GC_FUNC(), jump & 0x00ff, line);
    return offset;
}

void patchJump(Builder* builder, int patch) {
    assert(patch < UINT16_MAX && "jump offset too long");
    int current = GC_FUNC()->code.count - 2;
    int offset = current - patch;
    
    uint16_t jump = offset;
    
    GC_FUNC()->code.data[patch++] = (jump >> 8) & 0x00ff;
    GC_FUNC()->code.data[patch++] = jump & 0x00ff;
}

OrbitCode instSelect(Builder* builder, OrbitTokenKind op, const OrbitAST* lhs, const OrbitAST* rhs) {
    for(int i = 0; i < builder->selector.count; ++i) {
        OpSelectData data = builder->selector.data[i];
        if(op == data.op
           && orbitASTTypeEqualsPrimitive(lhs->type, data.lhsType)
           && orbitASTTypeEqualsPrimitive(rhs->type, data.rhsType)) {
            return data.instruction;
        }
    }
    // TODO: throw error
    return OP_return;
}
