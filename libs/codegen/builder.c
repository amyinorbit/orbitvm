//===--------------------------------------------------------------------------------------------===
// fn.c - Implementation of the Orbit Bytecode fn
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

static int32_t currentLine(const Function* fn) {
    return 1; // TODO: implementation (call into source::physicalLoc)
}

void contextInit(Codegen* ctx, OrbitGC* gc) {
    ctx->gc = gc;
    ctx->ast = NULL;
    ctx->fn = NULL;
    orbitSelectorArrayInit(&ctx->selector);

    // ctx->module = orbitModuleNew(ctx->gc);
    // orbitGCPush(ctx->gc, (OrbitObject*)ctx->module);

#define BINARY_OP(T, op, code) ((OpSelectData){(op), ORBIT_AST_TYPEEXPR_##T, ORBIT_AST_TYPEEXPR_##T, (code)})

    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(INT, ORBIT_TOK_PLUS, OP_iadd));
    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(INT, ORBIT_TOK_MINUS, OP_isub));
    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(INT, ORBIT_TOK_STAR, OP_imul));
    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(INT, ORBIT_TOK_SLASH, OP_idiv));

    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(INT, ORBIT_TOK_EQEQ, OP_ieq));
    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(INT, ORBIT_TOK_LT, OP_ilt));
    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(INT, ORBIT_TOK_GT, OP_igt));
    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(INT, ORBIT_TOK_LTEQ, OP_ilteq));
    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(INT, ORBIT_TOK_GT, OP_igteq));

    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(FLOAT, ORBIT_TOK_PLUS, OP_fadd));
    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(FLOAT, ORBIT_TOK_MINUS, OP_fsub));
    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(FLOAT, ORBIT_TOK_STAR, OP_fmul));
    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(FLOAT, ORBIT_TOK_SLASH, OP_fdiv));

    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(FLOAT, ORBIT_TOK_EQEQ, OP_feq));
    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(FLOAT, ORBIT_TOK_LT, OP_flt));
    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(FLOAT, ORBIT_TOK_GT, OP_fgt));
    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(FLOAT, ORBIT_TOK_LTEQ, OP_flteq));
    orbitSelectorArrayWrite(gc, &ctx->selector, BINARY_OP(FLOAT, ORBIT_TOK_GT, OP_fgteq));

#undef BINARY_OP
}

void contextDeinit(Codegen* ctx) {
    orbitSelectorArrayDeinit(ctx->gc, &ctx->selector);
}

#define GC(fn) ((fn)->context->gc)
// #define MODULE(fn) ((fn)->context->module->functions)
#define GC_FUNC() fn->impl
#define SEL(fn) ((fn)->context->selector)

void openFunctionGC(Codegen* gen, Function* fn, OCStringID signature, OrbitFunction* impl) {
    assert(fn && "cannot open a codegen function without a fn");
    fn->parent = gen->fn;
    fn->impl = impl;
    fn->context = gen;

    fn->localCount = 0;
    fn->maxLocals = 0;

    // TODO: here we need to handle the case of the root function, that gets declared in nothing (global)
    if(gen->fn) {
        fn->slot = localVariable(gen->fn, signature);
        orbitGCPop(gen->gc);
    } else {
        fn->slot = 0;
    }

    fn->localCount = 0;
    fn->maxLocals = 0;

    gen->fn = fn;
    orbitGCPush(gen->gc, (OrbitObject*)gen->fn->impl);
}

void openFunction(Codegen* gen, Function* fn, OCStringID signature) {
    assert(fn && "cannot open a codegen function without a fn");
    openFunctionGC(gen, fn, signature, orbitFunctionNew(gen->gc));
}

void finishParams(Function* fn) {
    GC_FUNC()->arity = fn->localCount;
}

OrbitFunction* closeFunction(Function* fn) {
    assert(fn && "cannot close a codegen function without a fn");
    GC_FUNC()->locals = fn->maxLocals;
    if(!GC_FUNC()->code.count) {
        emitInst(fn, OP_return);
    }
    OrbitFunction* compiled = fn->impl;
    fn->context->fn = fn->parent;

    orbitGCPop(fn->context->gc);
    if(fn->context->fn) {
        emitConstInst(fn->context->fn, OP_const, ORBIT_VALUE_REF(fn->impl));
        emitInst8(fn->context->fn, OP_store_local, fn->slot);
        orbitGCPush(fn->context->gc, (OrbitObject*)fn->context->fn->impl);
    }
    return compiled;
}


int openScope(Function* fn) {
    return fn->localCount;
}

void dropScope(Function* fn, int stack) {
    fn->localCount = stack;
}

int localVariable(Function* fn, OCStringID name) {
    assert(fn && "cannot locate a codegen variable without a fn");
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

int findConstant(OrbitValueArray* constants, OrbitValue value) {
    for(int i = 0; i < constants->count; ++i) {
        if(orbitValueEquals(constants->data[i], value)) return i;
    }
    return -1;
}

int offset(Function* fn) {
    return GC_FUNC()->code.count;
}


uint8_t emitConstant(Function* fn, OrbitValue value) {
    OrbitValueArray* constants = &GC_FUNC()->constants;;
    int existing = findConstant(constants, value);
    if(existing != -1) return (uint8_t)existing;

    assert(constants->count + 1 < UINT8_MAX && "only 256 constants are allowed in a function");
    orbitValueArrayWrite(GC(fn), constants, value);
    return (uint8_t)(constants->count-1);
}

int emitInst(Function* fn, OrbitCode code) {
    orbitFunctionWrite(GC(fn), GC_FUNC(), code, currentLine(fn));
    return GC_FUNC()->code.count - 1;
}

int emitInst8(Function* fn, OrbitCode code, uint8_t param) {
    int offset = emitInst(fn, code);
    orbitFunctionWrite(GC(fn), GC_FUNC(), param, currentLine(fn));
    return offset;
}

int emitConstInst(Function* fn, OrbitCode code, OrbitValue value) {
    int offset = emitInst(fn, code);
    uint8_t constantIndex = emitConstant(fn, value);
    orbitFunctionWrite(GC(fn), GC_FUNC(), constantIndex, currentLine(fn));
    return offset;
}

int emitLocalInst(Function* fn, OrbitCode code, OCStringID name) {
    int offset = emitInst(fn, code);
    uint8_t localIndex = (uint8_t)localVariable(fn, name);
    orbitFunctionWrite(GC(fn), GC_FUNC(), localIndex, currentLine(fn));
    return offset;
}

int emitJump(Function* fn, OrbitCode code) {
    int line = currentLine(fn);
    emitInst(fn, code);
    int patchOffset = GC_FUNC()->code.count;
    orbitFunctionWrite(GC(fn), GC_FUNC(), 0xff, line);
    orbitFunctionWrite(GC(fn), GC_FUNC(), 0xff, line);
    return patchOffset;
}

int emitRJump(Function* fn, OrbitCode code, int target) {
    int line = currentLine(fn);
    int offset = emitInst(fn, code);
    int current = GC_FUNC()->code.count + 2; // To account for the two byte jump offset
    uint16_t jump = current - target;

    orbitFunctionWrite(GC(fn), GC_FUNC(), (jump >> 8) & 0x00ff, line);
    orbitFunctionWrite(GC(fn), GC_FUNC(), jump & 0x00ff, line);
    return offset;
}

void patchJump(Function* fn, int patch) {
    assert(patch < UINT16_MAX && "jump offset too long");
    int current = GC_FUNC()->code.count - 2;
    int offset = current - patch;

    uint16_t jump = offset;

    GC_FUNC()->code.data[patch++] = (jump >> 8) & 0x00ff;
    GC_FUNC()->code.data[patch++] = jump & 0x00ff;
}

OrbitCode instSelect(Function* fn, OrbitTokenKind op, const OrbitAST* lhs, const OrbitAST* rhs) {
    for(int i = 0; i < SEL(fn).count; ++i) {
        OpSelectData data = SEL(fn).data[i];
        if(op == data.op
           && orbitASTTypeEqualsPrimitive(lhs->type, data.lhsType)
           && orbitASTTypeEqualsPrimitive(rhs->type, data.rhsType)) {
            return data.instruction;
        }
    }
    // TODO: throw error
    return OP_return;
}
