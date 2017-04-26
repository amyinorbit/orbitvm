//
//  orbit_lexer.h
//  OrbitVM
//
//  Created by Cesar Parent on 2017-03-01.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#ifndef orbit_lexer_h
#define orbit_lexer_h
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <orbit/orbit_utf8.h>

// OCLexer intends to be reusable - we don't define the main lexin function
// here, but instead we have a function pointer to one implemented by the user
// (or in our case orbit_parser).

typedef struct _OCLexer OCLexer;
typedef struct _OCToken OCToken;
typedef void (*lexFn)(OCLexer* lexer);

struct _OCToken {
    int             type;
    const char*     start;
    uint32_t        length;
};

/// A basic lexer
struct _OCLexer {
    /// The entire program's source - this allows nicer errors than fget()-ing
    /// every time we need the next character.
    const char*     source;
    uint64_t        sourceLength;
    
    /// Since we must handle UTF-8 source files (not every cahracter is a single
    /// byte), we can't just keep a pointer to the current character. We also
    /// have to store its unicode codepoint.
    const char*     currentPtr;
    codepoint_t     currentChar;
    
    uint64_t        line;
    uint64_t        column;
    
    /// The curent token
    OCToken         currentToken;
    
    /// The lexer function used to get the next token.
    lexFn           lex;
};

void lexer_init(OCLexer* lexer, const char* source, uint64_t length, lexFn lex);

/// Prints the line that [lexer] is currently lexing.
void lexer_printLine(FILE* out, OCLexer* lexer);

codepoint_t lexer_currentChar(OCLexer* lexer);

/// Moves [lexer] to the next unicode character from its source.
void lexer_nextChar(OCLexer* lexer);

/// Fetches the next token from the source.
void lexer_next(OCLexer* lexer);

OCToken* lexer_current(OCLexer* lexer);

void lexer_makeToken(OCLexer* lexer, int type, const char* start, uint64_t length);

#endif /* orbit_lexer_h_ */
