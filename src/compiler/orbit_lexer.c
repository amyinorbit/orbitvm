//
//  orbit_lexer.c
//  OrbitVM
//
//  Created by Amy Parent on 2017-03-01.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <string.h>
#include <ctype.h>
#include <orbit/orbit_utils.h>
#include "orbit_lexer.h"

void lexer_init(OCLexer* lexer, const char* source, uint64_t length) {
    OASSERT(lexer != NULL, "Null instance error");
    OASSERT(source != NULL, "Null source pointer");
    
    lexer->source = source;
    lexer->sourceLength = length;
    
    lexer->currentPtr = source;
    lexer->currentChar = 0;
    
    lexer->string.buffer = NULL;
    lexer->string.length = 0;
    lexer->string.capacity = 0;
    
    lexer->currentToken.type = 0;
    lexer->currentToken.start = NULL;
    lexer->currentToken.length = 0;
    
    //_nextChar(lexer);
}

void lexer_printLine(FILE* out, OCLexer* lexer) {
    OASSERT(lexer != NULL, "Null instance error");
}

static codepoint_t _nextChar(OCLexer* lexer) {
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

static inline codepoint_t _next(OCLexer* lexer) {
    uint64_t remaining = lexer->sourceLength - (lexer->currentPtr - lexer->source);
    return utf8_getCodepoint(lexer->currentPtr, remaining);
}

static void _makeToken(OCLexer* lexer, int type) {
    OASSERT(lexer != NULL, "Null instance error");
    lexer->currentToken.type = type;
    lexer->currentToken.start = lexer->tokenStart;
    lexer->currentToken.length = lexer->currentPtr - lexer->tokenStart;
}

static bool _match(OCLexer* lexer, codepoint_t c) {
    if(_next(lexer) != c) { return false; }
    _nextChar(lexer);
    return true;
}

static void _twoChars(OCLexer* lexer, codepoint_t c, int match, int nomatch) {
    _makeToken(lexer, _match(lexer, c) ? match : nomatch);
}


// MARK: - LEXER START

// A list of all the reserved orbit keywords:
static const struct _kw {
    const char* name;
    int         type;
} _keywords[] = {
    {"fuc",     TOKEN_FUN},
    {"var",     TOKEN_VAR},
    {"val",     TOKEN_VAL},
    {"maybe",   TOKEN_MAYBE},
    {"type",    TOKEN_TYPE},
    {"return",  TOKEN_RETURN},
    {"for",     TOKEN_FOR},
    {"while",   TOKEN_WHILE},
    {"break",   TOKEN_BREAK},
    {"init",    TOKEN_INIT},
    {"fail",    TOKEN_FAIL},
    {"range",   TOKEN_RANGE},
    {"Number",  TOKEN_NUMBER},
    {"Bool",    TOKEN_BOOL},
    {"String",  TOKEN_STRING},
    {"Nil",     TOKEN_NIL},
    {"Any",     TOKEN_ANY},
    {NULL, 0}
};

static void _lexIdentifier(OCLexer* lexer) {
    while(utf8_isIdentifier(_next(lexer))) {
        _nextChar(lexer);
    }
    
    int type = TOKEN_IDENTIFIER;
    uint32_t length = lexer->currentPtr - lexer->tokenStart;
    for(uint16_t i = 0; _keywords[i].name != NULL; ++i) {
        if(memcmp(_keywords[i].name, lexer->tokenStart, length) == 0) {
            type = _keywords[i].type;
            break;
        }
    }
    _makeToken(lexer, type);
}

static void _stringReserve(OCLexer* lexer) {
    if(lexer->string.capacity <= 0) { lexer->string.capacity = 32; }
    lexer->string.buffer = realloc(lexer->string.buffer, lexer->string.capacity);
}

static void _stringAppend(OCLexer* lexer, codepoint_t c) {
    int8_t size = utf8_codepointSize(c);
    if(size < 0) { 
        fprintf(stderr, "Invalid codepoint found in string: U+%X\n", c);
        return;
    }
    
    if(lexer->string.length + size >= lexer->string.capacity) {
        lexer->string.capacity *= 2;
        _stringReserve(lexer);
    }
    utf8_writeCodepoint(c, &lexer->string.buffer[lexer->string.length],
                           lexer->string.capacity - lexer->string.length);
    lexer->string.length += size;
}

static void _lexString(OCLexer* lexer) {
    lexer->tokenStart += 1;
    
    // String literals cannot be tokenised by solely pointing into the source
    // string, since there's the potential for 
    lexer->string.buffer = NULL;
    lexer->string.length = 0;
    _stringReserve(lexer);
    
    while(_next(lexer) != '\0') {
        codepoint_t c = _nextChar(lexer);
        if(c == '"') { break; }
        if(c == '\\') {
            c = _nextChar(lexer);
            switch(c) {
                case '\\': _stringAppend(lexer, '\\'); break;
                case 'a':  _stringAppend(lexer, '\a'); break;
                case 'b':  _stringAppend(lexer, '\b'); break;
                case 'f':  _stringAppend(lexer, '\f'); break;
                case 'n':  _stringAppend(lexer, '\n'); break;
                case 'r':  _stringAppend(lexer, '\r'); break;
                case 't':  _stringAppend(lexer, '\t'); break;
                case 'v':  _stringAppend(lexer, '\v'); break;
                case '"':  _stringAppend(lexer, '\"'); break;
                default:
                    // TODO: signal error
                    break;
            }
        } else {
            _stringAppend(lexer, c);
        }
    }
    lexer->currentToken.type = TOKEN_STRING_LITERAL;
    lexer->currentToken.start = lexer->string.buffer;
    lexer->currentToken.length = lexer->string.length;
}

static inline bool isDigit(codepoint_t c) {
    return c >= '0' && c <= '9';
}

static void _lexNumber(OCLexer* lexer) {
    
    int type = TOKEN_INTEGER_LITERAL;
    
    while(isDigit(_next(lexer))) {
        _nextChar(lexer);
    }
    if(_match(lexer, '.') && isDigit(_next(lexer))) {
        type = TOKEN_FLOAT_LITERAL;
        while(isDigit(_next(lexer))) {
            _nextChar(lexer);
        }
    }
    _makeToken(lexer, type);
}

void lexer_nextToken(OCLexer* lexer) {
    OASSERT(lexer != NULL, "Null instance error");
    if(lexer->currentToken.type == TOKEN_EOF) { return; }
    
    while(_next(lexer) != '\0') {
        lexer->tokenStart = lexer->currentPtr;
        codepoint_t c = _nextChar(lexer);
        switch(c) {
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
                if(_match(lexer, '>')) {
                    _makeToken(lexer, TOKEN_ARROW);
                } else {
                    _twoChars(lexer, '=', TOKEN_MINUSEQ, TOKEN_MINUS);
                }
                return;
            
            case '<':
                if(_match(lexer, '<')) {
                    _makeToken(lexer, TOKEN_LTLT);
                } else {
                    _twoChars(lexer, '=', TOKEN_LTEQ, TOKEN_LT);
                }
                return;
            
            case '>':
                if(_match(lexer, '>')) {
                    _makeToken(lexer, TOKEN_GTGT);
                } else {
                    _twoChars(lexer, '=', TOKEN_GTEQ, TOKEN_GT);
                }
                return;
            
            case '/':
                if(_match(lexer, '/')) {
                    // TODO: skip comments
                } else {
                    _twoChars(lexer, '=', TOKEN_SLASH, TOKEN_SLASHEQ);
                }
                return;
                
            case '"':
                _lexString(lexer);
                return;
            
            // More complex tokens get parsed here
            default:
                if(utf8_isIdentifierHead(c)) {
                    _lexIdentifier(lexer);
                }
                else if(isDigit(c)) {
                    _lexNumber(lexer);
                }
                else {
                    _makeToken(lexer, TOKEN_INVALID);
                }
                return;
        }
    }
    lexer->currentToken.type = TOKEN_EOF;
    lexer->currentToken.length = 0;
    lexer->currentToken.start = NULL;
}
