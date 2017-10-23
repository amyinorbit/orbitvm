//
//  orbit/parser/lexer.h
//  Orbit - Parser
//
//  Created by Amy Parent on 2017-03-01.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_lexer_h
#define orbit_lexer_h
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <orbit/source/source.h>
#include <orbit/source/tokens.h>
#include <orbit/utils/assert.h>
#include <orbit/utils/utf8.h>

typedef struct _OCLexer OCLexer;


/// A basic lexer
struct _OCLexer {
    /// The entire program's source - this allows nicer errors than fget()-ing
    /// every time we need the next character.
    OCSource        source;
    
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
    
    /// Buffer used when lexin string literals
    struct {
        char*       buffer;
        uint64_t    length;
        uint64_t    capacity;
    } string;
    
    /// We keep track of where the token being built starts.
    const char*     tokenStart;
    
    /// The curent token
    OCToken         currentToken;
};

void lexer_init(OCLexer* lexer, OCSource source);

/// Prints the line that [lexer] is currently lexing.
void lexer_printLine(FILE* out, OCLexer* lexer);

/// Fetches the next token from the source.
void lexer_nextToken(OCLexer* lexer);

#endif /* orbit_lexer_h_ */
