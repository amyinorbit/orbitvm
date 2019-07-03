//===--------------------------------------------------------------------------------------------===
// codegen.c - Implementation of the main code generation functions
// This source is part of Orbit
//
// Created on 2019-07-03 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/codegen/codegen.h>
#include <orbit/rt2/opcode.h>
#include <assert.h>

typedef struct {
    OrbitGC* gc;
    OrbitChunk* chunk;
    OrbitASTContext* context;
} Builder;

uint8_t emitConstant(Builder* builder, OrbitValue value) {
    //for()
}

void codegen(Builder* builder) {
    
}

void orbit_codegen(OrbitGC* gc, OrbitChunk* chunk, OrbitASTContext* context) {
    assert(gc && "null garbage collector error");
    assert(chunk && "null code chunk error");
    assert(context && "null AST context error");
}
