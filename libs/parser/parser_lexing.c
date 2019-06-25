//===--------------------------------------------------------------------------------------------===
// orbit/parser/lexer.c
// This source is part of Orbit - Parser
//
// Created on 2017-03-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
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
#include <orbit/utils/wcwidth.h>

#include "parser_private.h"

static inline char* _start(OCParser* parser) {
    return source(parser)->bytes;
}

static inline char* _end(OCParser* parser) {
    return source(parser)->bytes + source(parser)->length;
}

static inline uint64_t _remaining(OCParser* parser) {
    return source(parser)->length  - (parser->currentPtr - _start(parser));
}

OrbitSLoc lexer_loc(OCParser* parser) {
    return ORBIT_SLOC_MAKE(parser->currentPtr - _start(parser));
}

static codepoint_t _nextChar(OCParser* parser) {
    assert(parser != NULL && "Null instance error");
    if(!parser->currentPtr) { return parser->currentChar = '\0'; }
    
    parser->currentChar = utf8_getCodepoint(parser->currentPtr, _remaining(parser));
    
    // advance the current character pointer.
    int8_t size = utf8_codepointSize(parser->currentChar);
    if(size > 0 && parser->currentChar != 0) {
        parser->currentPtr += size;
    }
    
    if(parser->currentChar == '\n') {
        parser->isStartOfLine = true;
    }
    
    return parser->currentChar;
}

static inline codepoint_t _next(OCParser* parser) {
    return utf8_getCodepoint(parser->currentPtr, _remaining(parser));
}

static inline codepoint_t _next2(OCParser* parser) {
    uint64_t remaining = _remaining(parser);
    codepoint_t next = utf8_getCodepoint(parser->currentPtr, remaining);
    int8_t nextSize = utf8_codepointSize(next);
    remaining -= nextSize;
    return utf8_getCodepoint(parser->currentPtr + nextSize, remaining);
}

static void _makeToken(OCParser* parser, int type) {
    assert(parser != NULL && "Null instance error");
    
    parser->currentToken.kind = type;
    parser->currentToken.sourceLoc = ORBIT_SLOC_MAKE(parser->tokenStart - _start(parser));
    parser->currentToken.length = parser->currentPtr - parser->tokenStart;

    parser->currentToken.isStartOfLine = parser->isStartOfLine;
    parser->currentToken.parsedStringLiteral = orbit_invalidStringID;
    
    // We reset the start of line marker after a token is produced.
    parser->isStartOfLine = false;
}

static bool _match(OCParser* parser, codepoint_t c) {
    if(_next(parser) != c) { return false; }
    _nextChar(parser);
    return true;
}

static void _twoChars(OCParser* lexer, codepoint_t c, int match, int nomatch) {
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

static void _lexIdentifier(OCParser* parser) {
    while(utf8_isIdentifier(_next(parser))) {
        _nextChar(parser);
    }
    
    int type = ORBIT_TOK_IDENTIFIER;
    uint32_t length = parser->currentPtr - parser->tokenStart;
    for(uint16_t i = 0; _keywords[i].name != NULL; ++i) {
        if(_keywords[i].length == length &&
           memcmp(_keywords[i].name, parser->tokenStart, length) == 0) {
            type = _keywords[i].type;
            break;
        }
    }
    _makeToken(parser, type);
}

static void _lexString(OCParser* parser) {
    // String literals cannot be tokenised by solely pointing into the source
    // string, we need to account for escape sequences.
    orbit_stringBufferReset(&parser->literalBuffer);
    
    for(;;) {
        codepoint_t c = _nextChar(parser);
        
        if(c == '"') {
            break;
        }
        else if(c == '\0') {
            orbit_diagError(
                &parser->context->diagnostics,
                lexer_loc(parser), "unterminated string literal", 0
            );
            break;
        }
        else if(c == '\\') {
            c = _nextChar(parser);
            switch(c) {
                case '\\': orbit_stringBufferAppend(&parser->literalBuffer, '\\'); break;
                case 'a':  orbit_stringBufferAppend(&parser->literalBuffer, '\a'); break;
                case 'b':  orbit_stringBufferAppend(&parser->literalBuffer, '\b'); break;
                case 'f':  orbit_stringBufferAppend(&parser->literalBuffer, '\f'); break;
                case 'n':  orbit_stringBufferAppend(&parser->literalBuffer, '\n'); break;
                case 'r':  orbit_stringBufferAppend(&parser->literalBuffer, '\r'); break;
                case 't':  orbit_stringBufferAppend(&parser->literalBuffer, '\t'); break;
                case 'v':  orbit_stringBufferAppend(&parser->literalBuffer, '\v'); break;
                case '"':  orbit_stringBufferAppend(&parser->literalBuffer, '\"'); break;
                default:
                    orbit_diagError(
                        &parser->context->diagnostics,
                        lexer_loc(parser), "invalid escape sequence in string literal", 0
                    );
                    break;
            }
        } else {
            orbit_stringBufferAppend(&parser->literalBuffer, c);
        }
    }
    
    _makeToken(parser, ORBIT_TOK_STRING_LITERAL);
    // Store the parsed string literal
    parser->currentToken.parsedStringLiteral = orbit_stringBufferIntern(&parser->literalBuffer);
}

static inline bool isDigit(codepoint_t c) {
    return c >= '0' && c <= '9';
}

static void _lexNumber(OCParser* parser) {
    
    int type = ORBIT_TOK_INTEGER_LITERAL;
    
    while(isDigit(_next(parser))) {
        _nextChar(parser);
    }
    if(_next(parser) == '.' && isDigit(_next2(parser))) {
        _nextChar(parser);
        type = ORBIT_TOK_FLOAT_LITERAL;
        while(isDigit(_next(parser))) {
            _nextChar(parser);
        }
    }
    _makeToken(parser, type);
}

static void _eatLineComment(OCParser* parser) {
    while(_next(parser) != '\n' && _next(parser) != '\0') {
        _nextChar(parser);
    }
}

static void _lexBlockComment(OCParser* parser) {
    int depth = 1;
    for(;;) {
        codepoint_t c = _nextChar(parser);
        switch(c) {
        case '*':
            c = _nextChar(parser);
            if(c == '/') { depth -= 1; }
            break;
        case '/':
            c = _nextChar(parser);
            if(c == '*') { depth += 1; }
            break;
        case '\0':
            orbit_diagError(
                &parser->context->diagnostics,
                lexer_loc(parser), "unterminated string literal", 0
            );
            return;
        default: break;
        }
        if(depth == 0) { return; }
    }
}

static void _updateTokenStart(OCParser* parser) {
    parser->tokenStart = parser->currentPtr;
    parser->currentToken.source = source(parser); // TODO: nuke when OrbitToken::source is removed
}

void orbit_parserNextToken(OCParser* parser) {
    assert(parser != NULL && "invalid parser given");
    if(parser->currentToken.kind == ORBIT_TOK_EOF) { return; }
    
    while(_next(parser) != '\0' && parser->currentPtr < _end(parser)) {
        
        _updateTokenStart(parser);
        codepoint_t c = _nextChar(parser);
        
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
            case ';': _makeToken(parser, ORBIT_TOK_SEMICOLON);return;
            
            case '{': _makeToken(parser, ORBIT_TOK_LBRACE);   return;
            case '}': _makeToken(parser, ORBIT_TOK_RBRACE);   return;
            case '[': _makeToken(parser, ORBIT_TOK_LBRACKET); return;
            case ']': _makeToken(parser, ORBIT_TOK_RBRACKET); return;
            case '(': _makeToken(parser, ORBIT_TOK_LPAREN);   return;
            case ')': _makeToken(parser, ORBIT_TOK_RPAREN);   return;
            case ':': _makeToken(parser, ORBIT_TOK_COLON);    return;
            case '.': _makeToken(parser, ORBIT_TOK_DOT);      return;
            case ',': _makeToken(parser, ORBIT_TOK_COMMA);    return;

            case '^': _makeToken(parser, ORBIT_TOK_CARET);    return;
            case '~': _makeToken(parser, ORBIT_TOK_TILDE);    return;
            case '%': _makeToken(parser, ORBIT_TOK_PERCENT);  return;
            case '?': _makeToken(parser, ORBIT_TOK_QUESTION); return;
            
            case '+': _twoChars(parser, '=', ORBIT_TOK_PLUSEQ, ORBIT_TOK_PLUS);  return;
            case '&': _twoChars(parser, '&', ORBIT_TOK_AMPAMP, ORBIT_TOK_AMP);   return;
            case '|': _twoChars(parser, '|', ORBIT_TOK_PIPEPIPE, ORBIT_TOK_PIPE);return;
            case '!': _twoChars(parser, '=', ORBIT_TOK_BANGEQ, ORBIT_TOK_BANG);  return;
            
            case '=':
                if(_match(parser, '=')) {
                    _makeToken(parser, ORBIT_TOK_EQEQ);
                } else {
                    _twoChars(parser, '>', ORBIT_TOK_THEN, ORBIT_TOK_EQUALS);
                }
                return;
            
            case '*':
                if(_match(parser, '*')) {
                    _makeToken(parser, ORBIT_TOK_STARSTAR);
                } else {
                    _twoChars(parser, '=', ORBIT_TOK_STAREQ, ORBIT_TOK_STAR);
                }
                return;
            
            case '/':
                if(_match(parser, '/')) {
                    _eatLineComment(parser);
                } else if(_match(parser, '*')) {
                    _lexBlockComment(parser);
                }
                else {
                    _twoChars(parser, '=', ORBIT_TOK_SLASHEQ, ORBIT_TOK_SLASH);
                    return;
                }
                break;
            
            case '-':
                if(_match(parser, '>')) {
                    _makeToken(parser, ORBIT_TOK_ARROW);
                } else {
                    _twoChars(parser, '=', ORBIT_TOK_MINUSEQ, ORBIT_TOK_MINUS);
                }
                return;
            
            case '<':
                if(_match(parser, '<')) {
                    _makeToken(parser, ORBIT_TOK_LTLT);
                } else {
                    _twoChars(parser, '=', ORBIT_TOK_LTEQ, ORBIT_TOK_LT);
                }
                return;
            
            case '>':
                if(_match(parser, '>')) {
                    _makeToken(parser, ORBIT_TOK_GTGT);
                } else {
                    _twoChars(parser, '=', ORBIT_TOK_GTEQ, ORBIT_TOK_GT);
                }
                return;
            
            case '#':
                _eatLineComment(parser);
                break;
                
            case '"':
                _lexString(parser);
                return;
            
            // More complex tokens get parsed here
            default:
                if(utf8_isIdentifierHead(c)) {
                    _lexIdentifier(parser);
                }
                else if(isDigit(c)) {
                    _lexNumber(parser);
                }
                else {
                    orbit_diagError(
                        &parser->context->diagnostics,
                        lexer_loc(parser), "invalid character", 0
                    );
                    _makeToken(parser, ORBIT_TOK_INVALID);
                }
                return;
        }
    }
    parser->currentToken.kind = ORBIT_TOK_EOF;
    parser->currentToken.length = 0;
    parser->currentToken.sourceLoc = ORBIT_SLOC_MAKE(0);
}
