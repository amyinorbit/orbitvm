//===--------------------------------------------------------------------------------------------===
// orbit/parser/parser_private.c
// This source is part of Orbit - Parser
//
// Created on 2017-03-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include "parser_private.h"

void orbit_parserInit(OCParser* parser, OrbitASTContext* context) {
    assert(parser && "cannot initialise null parser");
    assert(context && "cannot use null ast context");
    
    parser->context = context;
    parser->recovering = false;
    
    parser->currentPtr = context->source.bytes;
    parser->currentChar = 0;
    parser->isStartOfLine = true;
    
    orbit_stringBufferInit(&parser->literalBuffer, 128);
    
    parser->currentToken.kind = 0;
    parser->currentToken.sourceLoc = ORBIT_SLOC_MAKE(0);
    parser->currentToken.length = 0;
    // parser->currentToken.source = &context->source;
}

void orbit_parserDeinit(OCParser* parser) {
    assert(parser && "cannot deinit null parser");
    orbit_stringBufferDeinit(&parser->literalBuffer);
}
