//
//  orbit_lexer.c
//  OrbitVM
//
//  Created by Amy Parent on 2017-03-01.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include "orbit_lexer.h"
#include <orbit/orbit_utils.h>

void lexer_init(OCLexer* lexer, const char* source, uint64_t length, lexFn lex) {
    OASSERT(lexer != NULL, "Null instance error");
    OASSERT(source != NULL, "Null source pointer");
    
    lexer->source = source;
    lexer->sourceLength = length;
    
    lexer->currentPtr = source;
    lexer->currentChar = 0;
    
    lexer->currentToken.type = 0;
    lexer->currentToken.start = NULL;
    lexer->currentToken.length = 0;
    
    lexer->lex = lex;
    
    lexer_nextChar(lexer);
}

void lexer_printLine(FILE* out, OCLexer* lexer) {
    OASSERT(lexer != NULL, "Null instance error");
    //if(!lexer->line) { return; }
    //fprintf(out, "%*.s", (int)lexer->lineLength, lexer->line);
}

void lexer_nextChar(OCLexer* lexer) {
    OASSERT(lexer != NULL, "Null instance error");
    if(!lexer->currentPtr) { return 0; }
    
    uint64_t remaining = lexer->sourceLength - (lexer->currentPtr - lexer->source);
    lexer->currentChar = utf8_getCodepoint(lexer->currentPtr, remaining);
    
    // advance the current character pointer.
    int8_t size = utf8_codepointSize(lexer->currentChar);
    if(size > 0 && lexer->currentChar != 0) {
        lexer->currentPtr += size;
    }
    return lexer->currentChar;
}

OCToken* lexer_next(OCLexer* lexer) {
    OASSERT(lexer != NULL, "Null instance error");
    // Main lexing state machine goes here
    return &lexer->currentToken;
}

void lexer_makeToken(OCLexer* lexer, int type, const char* start, uint64_t length) {
    OASSERT(lexer != NULL, "Null instance error");
    lexer->currentToken.type = type;
    lexer->currentToken.start = start;
    lexer->currentToken.length = length;
}
