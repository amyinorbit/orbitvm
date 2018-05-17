//===--------------------------------------------------------------------------------------------===
// orbit/parser/lexer.c
// This source is part of Orbit - Parser
//
// Created on 2017-03-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <inttypes.h>
#include <orbit/ast/diag.h>
#include <orbit/csupport/console.h>
#include <orbit/parser/lexer.h>
#include <orbit/utils/wcwidth.h>

OrbitSLoc lexer_loc(OCLexer* lexer) {
    uint32_t offset = (lexer)->currentPtr - (lexer)->source->bytes;
    return (OrbitSLoc){.offset=offset, .valid=1};
}

void lexer_init(OCLexer* lexer, OrbitSource* source) {
    assert(lexer != NULL && "Null instance error");
    
    lexer->source = source;
    lexer->currentPtr = source->bytes;
    lexer->currentChar = 0;
    
    lexer->startOfLine = true;
    orbit_stringBufferInit(&lexer->buffer, 64);
    
    lexer->currentToken.kind = 0;
    lexer->currentToken.sourceLoc.offset = 0;
    lexer->currentToken.sourceLoc.valid = 1;
    lexer->currentToken.length = 0;
    lexer->currentToken.source = lexer->source;
}

void lexer_deinit(OCLexer* lexer) {
    assert(lexer != NULL && "Null instance error");
    orbit_stringBufferDeinit(&lexer->buffer);
}

static codepoint_t _nextChar(OCLexer* lexer) {
    assert(lexer != NULL && "Null instance error");
    if(!lexer->currentPtr) { return lexer->currentChar = '\0'; }
    
    uint64_t remaining = lexer->source->length - (lexer->currentPtr - lexer->source->bytes);
    lexer->currentChar = utf8_getCodepoint(lexer->currentPtr, remaining);
    
    // advance the current character pointer.
    int8_t size = utf8_codepointSize(lexer->currentChar);
    if(size > 0 && lexer->currentChar != 0) {
        lexer->currentPtr += size;
    }
    
    if(lexer->currentChar == '\n') {
        lexer->startOfLine = true;
    }
    
    return lexer->currentChar;
}

static inline codepoint_t _next(OCLexer* lexer) {
    uint64_t remaining = lexer->source->length - (lexer->currentPtr - lexer->source->bytes);
    return utf8_getCodepoint(lexer->currentPtr, remaining);
}

static inline codepoint_t _next2(OCLexer* lexer) {
    uint64_t remaining = lexer->source->length - (lexer->currentPtr - lexer->source->bytes);
    codepoint_t next = utf8_getCodepoint(lexer->currentPtr, remaining);
    int8_t nextSize = utf8_codepointSize(next);
    remaining -= nextSize;
    return utf8_getCodepoint(lexer->currentPtr + nextSize, remaining);
}

static void _makeToken(OCLexer* lexer, int type) {
    assert(lexer != NULL && "Null instance error");
    lexer->currentToken.kind = type;
    lexer->currentToken.sourceLoc.offset = lexer->tokenStart - lexer->source->bytes;
    lexer->currentToken.length = lexer->currentPtr - lexer->tokenStart;

    lexer->currentToken.isStartOfLine = lexer->startOfLine;
    lexer->currentToken.parsedStringLiteral = 0; // TODO: replace with invalid StringID constant
    
    // We reset the start of line marker after a token is produced.
    lexer->startOfLine = false;
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
    uint64_t    length;
    int         type;
} _keywords[] = {
    {"fun",     3,  ORBIT_TOK_FUN},
    {"var",     3,  ORBIT_TOK_VAR},
    {"const",   5,  ORBIT_TOK_CONST},
    {"maybe",   5,  ORBIT_TOK_MAYBE},
    {"type",    4,  ORBIT_TOK_TYPE},
    {"return",  6,  ORBIT_TOK_RETURN},
    {"for",     3,  ORBIT_TOK_FOR},
    {"in",      2,  ORBIT_TOK_IN},
    {"while",   5,  ORBIT_TOK_WHILE},
    {"break",   5,  ORBIT_TOK_BREAK},
    {"continue",7,  ORBIT_TOK_CONTINUE},
    {"if",      2,  ORBIT_TOK_IF},
    {"else",    4,  ORBIT_TOK_ELSE},
    {"init",    4,  ORBIT_TOK_INIT},
    {"fail",    4,  ORBIT_TOK_FAIL},
    {"range",   5,  ORBIT_TOK_RANGE},
    {"Number",  6,  ORBIT_TOK_NUMBER},
    {"Int",     3,  ORBIT_TOK_NUMBER},
    {"Float",   5,  ORBIT_TOK_NUMBER},
    {"Bool",    4,  ORBIT_TOK_BOOL},
    {"String",  6,  ORBIT_TOK_STRING},
    {"Nil",     3,  ORBIT_TOK_NIL},
    {"Void",    4,  ORBIT_TOK_VOID},
    {"Array",   5,  ORBIT_TOK_ARRAY},
    {"Map",     3,  ORBIT_TOK_MAP},
    {"Any",     3,  ORBIT_TOK_ANY},
    {NULL, 0}
};

static void _lexIdentifier(OCLexer* lexer) {
    while(utf8_isIdentifier(_next(lexer))) {
        _nextChar(lexer);
    }
    
    int type = ORBIT_TOK_IDENTIFIER;
    uint32_t length = lexer->currentPtr - lexer->tokenStart;
    for(uint16_t i = 0; _keywords[i].name != NULL; ++i) {
        if(_keywords[i].length == length &&
           memcmp(_keywords[i].name, lexer->tokenStart, length) == 0) {
            type = _keywords[i].type;
            break;
        }
    }
    _makeToken(lexer, type);
}

static void _lexString(OCLexer* lexer) {
    // String literals cannot be tokenised by solely pointing into the source
    // string, we need to account for escape sequences.
    orbit_stringBufferReset(&lexer->buffer);
    
    for(;;) {
        codepoint_t c = _nextChar(lexer);
        
        if(c == '"') {
            break;
        }
        else if(c == '\0') {
            orbit_diagEmitError(lexer_loc(lexer), "unterminated string literal", 0);
            break;
        }
        else if(c == '\\') {
            c = _nextChar(lexer);
            switch(c) {
                case '\\': orbit_stringBufferAppend(&lexer->buffer, '\\'); break;
                case 'a':  orbit_stringBufferAppend(&lexer->buffer, '\a'); break;
                case 'b':  orbit_stringBufferAppend(&lexer->buffer, '\b'); break;
                case 'f':  orbit_stringBufferAppend(&lexer->buffer, '\f'); break;
                case 'n':  orbit_stringBufferAppend(&lexer->buffer, '\n'); break;
                case 'r':  orbit_stringBufferAppend(&lexer->buffer, '\r'); break;
                case 't':  orbit_stringBufferAppend(&lexer->buffer, '\t'); break;
                case 'v':  orbit_stringBufferAppend(&lexer->buffer, '\v'); break;
                case '"':  orbit_stringBufferAppend(&lexer->buffer, '\"'); break;
                default:
                    orbit_diagEmitError(lexer_loc(lexer),
                                        "invalid escape sequence in string literal", 0);
                    break;
            }
        } else {
            orbit_stringBufferAppend(&lexer->buffer, c);
        }
    }
    
    _makeToken(lexer, ORBIT_TOK_STRING_LITERAL);
    // Store the parsed string literal
    lexer->currentToken.parsedStringLiteral = orbit_stringBufferIntern(&lexer->buffer);
}

static inline bool isDigit(codepoint_t c) {
    return c >= '0' && c <= '9';
}

static void _lexNumber(OCLexer* lexer) {
    
    int type = ORBIT_TOK_INTEGER_LITERAL;
    
    while(isDigit(_next(lexer))) {
        _nextChar(lexer);
    }
    if(_next(lexer) == '.' && isDigit(_next2(lexer))) {
        _nextChar(lexer);
        type = ORBIT_TOK_FLOAT_LITERAL;
        while(isDigit(_next(lexer))) {
            _nextChar(lexer);
        }
    }
    _makeToken(lexer, type);
}

static void _eatLineComment(OCLexer* lexer) {
    while(_next(lexer) != '\n' && _next(lexer) != '\0') {
        _nextChar(lexer);
    }
}

static void _lexBlockComment(OCLexer* lexer) {
    int depth = 1;
    for(;;) {
        codepoint_t c = _nextChar(lexer);
        switch(c) {
        case '*':
            c = _nextChar(lexer);
            if(c == '/') { depth -= 1; }
            break;
        case '/':
            c = _nextChar(lexer);
            if(c == '*') { depth += 1; }
            break;
        case '\0':
            orbit_diagEmitError(lexer_loc(lexer), "unterminated string literal", 0);
            return;
        default: break;
        }
        if(depth == 0) { return; }
    }
}

static void _updateTokenStart(OCLexer* lexer) {
    lexer->tokenStart = lexer->currentPtr;
    lexer->currentToken.source = lexer->source;
}

void lexer_nextToken(OCLexer* lexer) {
    assert(lexer != NULL && "Null instance error");
    if(lexer->currentToken.kind == ORBIT_TOK_EOF) { return; }
    
    while(_next(lexer) != '\0') {
        
        _updateTokenStart(lexer);
        codepoint_t c = _nextChar(lexer);
        
        switch(c) {
            // whitespace, we just loop
            case 0x0020:
            case 0x000d:
            case 0x0009:
            case 0x000b:
            case 0x000c:
            case 0x000a:
                break;
            
            // single character tokens
            case ';': _makeToken(lexer, ORBIT_TOK_SEMICOLON);return;
            
            case '{': _makeToken(lexer, ORBIT_TOK_LBRACE);   return;
            case '}': _makeToken(lexer, ORBIT_TOK_RBRACE);   return;
            case '[': _makeToken(lexer, ORBIT_TOK_LBRACKET); return;
            case ']': _makeToken(lexer, ORBIT_TOK_RBRACKET); return;
            case '(': _makeToken(lexer, ORBIT_TOK_LPAREN);   return;
            case ')': _makeToken(lexer, ORBIT_TOK_RPAREN);   return;
            case ':': _makeToken(lexer, ORBIT_TOK_COLON);    return;
            case '.': _makeToken(lexer, ORBIT_TOK_DOT);      return;
            case ',': _makeToken(lexer, ORBIT_TOK_COMMA);    return;

            case '^': _makeToken(lexer, ORBIT_TOK_CARET);    return;
            case '~': _makeToken(lexer, ORBIT_TOK_TILDE);    return;
            case '%': _makeToken(lexer, ORBIT_TOK_PERCENT);  return;
            case '?': _makeToken(lexer, ORBIT_TOK_QUESTION); return;
            
            case '+': _twoChars(lexer, '=', ORBIT_TOK_PLUSEQ, ORBIT_TOK_PLUS);  return;
            case '&': _twoChars(lexer, '&', ORBIT_TOK_AMPAMP, ORBIT_TOK_AMP);   return;
            case '|': _twoChars(lexer, '|', ORBIT_TOK_PIPEPIPE, ORBIT_TOK_PIPE);return;
            case '!': _twoChars(lexer, '=', ORBIT_TOK_BANGEQ, ORBIT_TOK_BANG);  return;
            case '=': _twoChars(lexer, '=', ORBIT_TOK_EQEQ, ORBIT_TOK_EQUALS);  return;
            
            case '*':
                if(_match(lexer, '*')) {
                    _makeToken(lexer, ORBIT_TOK_STARSTAR);
                } else {
                    _twoChars(lexer, '=', ORBIT_TOK_STAREQ, ORBIT_TOK_STAR);
                }
                return;
            
            case '/':
                if(_match(lexer, '/')) {
                    _eatLineComment(lexer);
                } else if(_match(lexer, '*')) {
                    _lexBlockComment(lexer);
                }
                else {
                    _twoChars(lexer, '=', ORBIT_TOK_SLASHEQ, ORBIT_TOK_SLASH);
                    return;
                }
                break;
            
            case '-':
                if(_match(lexer, '>')) {
                    _makeToken(lexer, ORBIT_TOK_ARROW);
                } else {
                    _twoChars(lexer, '=', ORBIT_TOK_MINUSEQ, ORBIT_TOK_MINUS);
                }
                return;
            
            case '<':
                if(_match(lexer, '<')) {
                    _makeToken(lexer, ORBIT_TOK_LTLT);
                } else {
                    _twoChars(lexer, '=', ORBIT_TOK_LTEQ, ORBIT_TOK_LT);
                }
                return;
            
            case '>':
                if(_match(lexer, '>')) {
                    _makeToken(lexer, ORBIT_TOK_GTGT);
                } else {
                    _twoChars(lexer, '=', ORBIT_TOK_GTEQ, ORBIT_TOK_GT);
                }
                return;
            
            case '#':
                _eatLineComment(lexer);
                break;
                
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
                    orbit_diagEmitError(lexer_loc(lexer), "invalid character", 0);
                    _makeToken(lexer, ORBIT_TOK_INVALID);
                }
                return;
        }
    }
    lexer->currentToken.kind = ORBIT_TOK_EOF;
    lexer->currentToken.length = 0;
    lexer->currentToken.sourceLoc.offset = 0;
}
