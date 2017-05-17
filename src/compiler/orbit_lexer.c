//
//  orbit_lexer.c
//  OrbitVM
//
//  Created by Amy Parent on 2017-03-01.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include "orbit_lexer.h"
#include <compiler/orbit_tokens.h>
#include <orbit/orbit_utils.h>

void lexer_init(OCLexer* lexer, const char* source, uint64_t length) {
    OASSERT(lexer != NULL, "Null instance error");
    OASSERT(source != NULL, "Null source pointer");
    
    lexer->source = source;
    lexer->sourceLength = length;
    
    lexer->currentPtr = source;
    lexer->currentChar = 0;
    
    lexer->currentToken.type = 0;
    lexer->currentToken.start = NULL;
    lexer->currentToken.length = 0;
    
    lexer_nextChar(lexer);
}

void lexer_printLine(FILE* out, OCLexer* lexer) {
    OASSERT(lexer != NULL, "Null instance error");
}

void lexer_nextChar(OCLexer* lexer) {
    OASSERT(lexer != NULL, "Null instance error");
    if(!lexer->currentPtr) { return; }
    
    uint64_t remaining = lexer->sourceLength - (lexer->currentPtr - lexer->source);
    lexer->currentChar = utf8_getCodepoint(lexer->currentPtr, remaining);
    
    // advance the current character pointer.
    int8_t size = utf8_codepointSize(lexer->currentChar);
    if(size > 0 && lexer->currentChar != 0) {
        lexer->currentPtr += size;
    }
    return;
}

static inline codepoint_t _current(OCLexer* lexer) {
    return lexer->currentChar;
}

static inline codepoint_t _next(OCLexer* lexer) {
    uint64_t remaining = lexer->sourceLength - (lexer->currentPtr - lexer->source);
    return utf8_getCodepoint(lexer->currentPtr, remaining);
}

static void _makeToken(OCLexer* lexer, int type) {
    OASSERT(lexer != NULL, "Null instance error");
    lexer->currentToken.type = type;
    lexer->currentToken.start = lexer->tokenStart;
    lexer->currentToken.length = lexer->tokenStart - lexer->currentPtr;
}

static bool _match(OCLexer* lexer, codepoint_t c) {
    if(_next(lexer) != c) { return false; }
    lexer_nextChar(lexer);
    return true;
}

static void _twoChars(OCLexer* lexer, codepoint_t c, int match, int nomatch) {
    _makeToken(lexer, _match(lexer, c) ? match : nomatch);
}

void lexer_next(OCLexer* lexer) {
    OASSERT(lexer != NULL, "Null instance error");
    
    // Main lexing state machine goes here
    lexer_nextChar(lexer);
    
    while(true) {
        lexer->tokenStart = lexer->currentPtr;
        
        switch(_current(lexer)) {
            // whitespace, we just loop
            case 0x0020:
            case 0x000d:
            case 0x0009:
            case 0x000b:
            case 0x000c:
                break;
            
            // single character tokens
            case '\n': _makeToken(lexer, TOKEN_NEWLINE); return;
            case ';': _makeToken(lexer, TOKEN_SEMICOLON);return;
            
            case '{': _makeToken(lexer, TOKEN_LBRACE);   return;
            case '}': _makeToken(lexer, TOKEN_RBRACE);   return;
            case '[': _makeToken(lexer, TOKEN_LBRACKET); return;
            case ']': _makeToken(lexer, TOKEN_RBRACKET); return;
            case '(': _makeToken(lexer, TOKEN_LPAREN);   return;
            case ')': _makeToken(lexer, TOKEN_RPAREN);   return;
            case ':': _makeToken(lexer, TOKEN_COLON);    return;
            case '.': _makeToken(lexer, TOKEN_DOT);      return;
            case ',': _makeToken(lexer, TOKEN_COMMA);    return;

            case '^': _makeToken(lexer, TOKEN_CARET);    return;
            case '~': _makeToken(lexer, TOKEN_TILDE);    return;
            case '%': _makeToken(lexer, TOKEN_PERCENT);  return;
            case '?': _makeToken(lexer, TOKEN_QUESTION); return;
            
            
            case '*': _twoChars(lexer, '=', TOKEN_STAREQ, TOKEN_STAR);  return;
            case '+': _twoChars(lexer, '=', TOKEN_PLUSEQ, TOKEN_PLUS);  return;
            case '&': _twoChars(lexer, '&', TOKEN_AMPAMP, TOKEN_AMP);   return;
            case '|': _twoChars(lexer, '|', TOKEN_PIPEPIPE, TOKEN_PIPE);return;
            case '!': _twoChars(lexer, '=', TOKEN_BANGEQ, TOKEN_BANG);  return;
            case '=': _twoChars(lexer, '=', TOKEN_EQEQ, TOKEN_EQUALS);  return;
            
            case '-':
                if(_next(lexer) == '>') {
                    _makeToken(lexer, TOKEN_ARROW);
                    return;
                }
                _twoChars(lexer, '=', TOKEN_MINUSEQ, TOKEN_MINUS);
                return;
            
            case '<':
                if(_next(lexer) == '<') {
                    _makeToken(lexer, TOKEN_LTLT);
                    return;
                }
                _twoChars(lexer, '=', TOKEN_LTEQ, TOKEN_LT);
                return;
            
            case '>':
                if(_next(lexer) == '>') {
                    _makeToken(lexer, TOKEN_GTGT);
                    return;
                }
                _twoChars(lexer, '=', TOKEN_GTEQ, TOKEN_GT);
                return;
            
            case '/':
                // TODO: skip comments
                if(_next(lexer) == '/') {
                    // TODO: skip comments
                } else {
                    _twoChars(lexer, '=', TOKEN_SLASH, TOKEN_SLASHEQ);
                    return;
                }
                break;
            
        }
    }
}
