//===--------------------------------------------------------------------------------------------===
// orbit/ast/context.h - Contexts are used to track all objects required during compilation
// This source is part of Orbit - AST
//
// Created on 2018-05-17 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <orbit/ast/context.h>

void orbit_astContextInit(OrbitASTContext* context) {
    assert(context != NULL && "Invalid AST context given");
    
    context->root = NULL;
    context->source.path = NULL;
    context->source.bytes = NULL;
    context->source.lineMap = NULL;
    context->source.length = 0;
    orbit_diagManagerInit(&context->diagnostics, &context->source);
}

void orbit_astContextDeinit(OrbitASTContext* context) {
    assert(context != NULL && "Invalid AST context given");
    ORCRELEASE(context->root);
    orbit_sourceDeinit(&context->source);
    orbit_diagManagerDeinit(&context->diagnostics);
}
