//===--------------------------------------------------------------------------------------------===
// orbit/parser/parser_private.h
// This source is part of Orbit - Parser
//
// Created on 2017-03-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_parser_parser_private_h
#define orbit_parser_parser_private_h
#include <orbit/ast/context.h>
#include <orbit/csupport/source.h>
#include <orbit/csupport/string.h>
#include <orbit/csupport/tokens.h>
#include <orbit/utils/utf8.h>

typedef struct _OCLexer OCLexer;
typedef struct _OCParser OCParser;

/// A basic lexer
struct _OCLexer {
    
    /// Since we must handle UTF-8 source files (not every cahracter is a single
    /// byte), we can't just keep a pointer to the current character. We also
    /// have to store its unicode codepoint.
    /// We also keep a pointer to the start of the current line, to make error
    /// printing easier.
    const char*     currentPtr;
    codepoint_t     currentChar;
    bool            startOfLine;
    
    /// Buffer used when lexing string literals
    OCStringBuffer  buffer;
    
    /// We keep track of where the token being built starts.
    const char*     tokenStart;
    
    /// The curent token
    OrbitToken      currentToken;
};

struct _OCParser {
    //OCLexer             lexer;
    OrbitASTContext*    context;
    bool                recovering;
    
    const char*         currentPtr;
    codepoint_t         currentChar;
    const char*         tokenStart;
    bool                isStartOfLine;
    
    OrbitToken          currentToken;
    OCStringBuffer      literalBuffer;
    
};

void orbitParserInit(OCParser* parser, OrbitASTContext* context);
void orbitParserDeinit(OCParser* parser);
void orbitParserNextToken(OCParser* parser);

static inline OrbitDiagManager* diag(OCParser* parser) {
    return &parser->context->diagnostics;
}

static inline OrbitSource* source(OCParser* parser) {
    return &parser->context->source;
}

#endif /* orbit_parser_parser_private_h */
