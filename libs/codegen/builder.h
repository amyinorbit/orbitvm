//===--------------------------------------------------------------------------------------------===
// builder.h - Builder is the API used to emit Orbit bytecode
// This source is part of Orbit
//
// Created on 2019-07-04 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_builder_h
#define orbit_builder_h
#include <orbit/ast/context.h>
#include <orbit/rt2/garbage.h>
#include <orbit/rt2/value.h>
#include <orbit/rt2/value_object.h>
#include <orbit/rt2/opcodes.h>
#include <orbit/rt2/buffer.h>
#include <orbit/rt2/memory.h>
#include <orbit/utils/memory.h>
#include <orbit/csupport/rcmap.h>
#include <orbit/common.h>

typedef struct {
    OrbitTokenKind op;
    ASTKind lhsType;
    ASTKind rhsType;
    OrbitCode instruction;
} OpSelectData;

DECLARE_BUFFER(Selector, OpSelectData);

typedef struct sFunction Function;
typedef struct sCodegen Codegen;

struct sCodegen {
    OrbitGC* gc;
    OrbitASTContext* ast;
    OrbitSelectorArray selector;
    Function* fn;
};

struct sFunction {
    Codegen* context;
    Function* parent;
    OrbitFunction* impl;

    // For the time being, functions are stored in local variable slots.
    // In the future, we probably want to change that so we can have proper overload and all.
    int slot;

    int localCount, maxLocals;
    OCStringID locals[256];
};

int local(Function* gen, OCStringID name);

void contextInit(Codegen* fn, OrbitGC* gc);
void contextDeinit(Codegen* fn);

int findConstant(OrbitValueArray* constants, OrbitValue value);
uint8_t emitConstant(Function* fn, OrbitValue value);

int openScope(Function* fn);
void dropScope(Function* fn, int stack);
int localVariable(Function* fn, OCStringID name);
// int localFunction(Function* fn, OCStringID name);

void openFunction(Codegen* gen, Function* fn, OCStringID name);
void finishParams(Function* fn);
void openFunctionGC(Codegen* gen, Function* fn, OCStringID name, OrbitFunction* impl);
OrbitFunction* closeFunction(Function* fn);

int offset(Function* fn);
int emitInst(Function* fn, OrbitCode code);
int emitInst8(Function* fn, OrbitCode code, uint8_t param);
int emitLocalInst(Function* fn, OrbitCode code, OCStringID name);
int emitConstInst(Function* fn, OrbitCode code, OrbitValue value);
int emitJump(Function* fn, OrbitCode code);
void patchJump(Function* fn, int patch);
int emitRJump(Function* fn, OrbitCode code, int target);

OrbitCode instSelect(Function* fn, OrbitTokenKind op, const OrbitAST* lhs, const OrbitAST* rhs);

#endif
