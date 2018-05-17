//===--------------------------------------------------------------------------------------------===
// orbit/parser/lexer.h
// This source is part of Orbit - Parser
//
// Created on 2017-03-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_lexer_h
#define orbit_lexer_h
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <orbit/csupport/source.h>
#include <orbit/csupport/string.h>
#include <orbit/csupport/tokens.h>
#include <orbit/utils/utf8.h>

typedef struct _OCLexer OCLexer;


/// A basic lexer
struct _OCLexer {
    /// The entire program's source - this allows nicer errors than fget()-ing
    /// every time we need the next character.
    OCSource*       source;
    
    /// Since we must handle UTF-8 source files (not every cahracter is a single
    /// byte), we can't just keep a pointer to the current character. We also
    /// have to store its unicode codepoint.
    /// We also keep a pointer to the start of the current line, to make error
    /// printing easier.
    const char*     linePtr;
    const char*     currentPtr;
    codepoint_t     currentChar;
    
    bool            startOfLine;
    uint64_t        line;
    uint64_t        column;
    
    /// Buffer used when lexing string literals
    OCStringBuffer  buffer;
    
    /// We keep track of where the token being built starts.
    const char*     tokenStart;
    
    /// The curent token
    OrbitToken         currentToken;
};

void lexer_init(OCLexer* lexer, OCSource* source);
void lexer_deinit(OCLexer* lexer);

/// Fetches the next token from the source.
void lexer_nextToken(OCLexer* lexer);

#endif /* orbit_lexer_h_ */
