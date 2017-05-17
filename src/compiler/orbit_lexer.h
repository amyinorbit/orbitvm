//
//  orbit_lexer.h
//  OrbitVM
//
//  Created by Amy Parent on 2017-03-01.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_lexer_h
#define orbit_lexer_h
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <orbit/orbit_utf8.h>

typedef struct _OCLexer OCLexer;
typedef struct _OCToken OCToken;

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
};

void lexer_init(OCLexer* lexer, const char* source, uint64_t length);

/// Prints the line that [lexer] is currently lexing.
void lexer_printLine(FILE* out, OCLexer* lexer);

/// Moves [lexer] to the next unicode character from its source.
codepoint_t lexer_nextChar(OCLexer* lexer);

/// Fetches the next token from the source.
OCToken* lexer_next(OCLexer* lexer);

void lexer_makeToken(OCLexer* lexer, int type, const char* start, uint64_t length);

#endif /* orbit_lexer_h_ */
