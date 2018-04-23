//===--------------------------------------------------------------------------------------------===
// orbit/source/tokens.h
// This source is part of Orbit - Source Handling
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_source_tokens_h
#define orbit_source_tokens_h

#include <stdint.h>
#include <stdbool.h>
#include <orbit/utils/string.h>
#include <orbit/source/source.h>

typedef struct _OCToken OCToken;

typedef enum  {
    // Bracket types
    // (){}[]
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    
    // Orbit Tokens
    // + - / * % ^ & | < > = >= <= == += ~ >> << && || != ! ?
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_SLASH,
    TOKEN_STAR,
    TOKEN_STARSTAR,
    TOKEN_PERCENT,
    TOKEN_CARET,
    TOKEN_TILDE,
    TOKEN_AMP,
    TOKEN_PIPE,
    TOKEN_BANG,
    TOKEN_QUESTION,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_EQUALS,
    TOKEN_LTEQ,
    TOKEN_GTEQ,
    TOKEN_EQEQ,
    TOKEN_PLUSEQ,
    TOKEN_MINUSEQ,
    TOKEN_STAREQ,
    TOKEN_SLASHEQ,
    TOKEN_BANGEQ,
    TOKEN_LTLT,
    TOKEN_GTGT,
    TOKEN_AMPAMP,
    TOKEN_PIPEPIPE,
    
    TOKEN_SEMICOLON,
    TOKEN_NEWLINE,
    
    // Punctuation:
    // :,.
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_ARROW,
    
    // Language objects
    TOKEN_INTEGER_LITERAL,
    TOKEN_FLOAT_LITERAL,
    //TOKEN_EXP_LITERAL, // later
    TOKEN_STRING_LITERAL,
    TOKEN_IDENTIFIER,
    
    // Language keywords
    TOKEN_FUN,
    TOKEN_VAR,
    TOKEN_CONST,
    TOKEN_MAYBE,
    TOKEN_TYPE,
    TOKEN_RETURN,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_IN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_INIT,
    TOKEN_FAIL,
    TOKEN_RANGE,
    
    // Types
    TOKEN_NUMBER,
    TOKEN_BOOL,
    TOKEN_STRING,
    TOKEN_NIL,
    TOKEN_VOID,
    TOKEN_ARRAY,
    TOKEN_MAP,
    TOKEN_ANY,
    
    TOKEN_EOF,
    TOKEN_INVALID,
    
} OCTokenKind;


struct _OCToken {
    OCTokenKind     kind;
    const OCSource* source;
    OCSourceLoc     sourceLoc;
    
    uint32_t        length;
    uint32_t        displayLength;
    bool            isStartOfLine;
    
    //OCString*       parsedStringLiteral;
    UTFConstString* parsedStringLiteral;
};

const char* source_tokenName(OCTokenKind token);
const char* source_tokenString(OCTokenKind token);
// OCString* source_stringFromToken(OCToken* token);
UTFConstString* source_stringFromToken(OCToken* token);

bool source_tokenEquals(OCToken* a, OCToken* b);

bool source_isBinaryOp(OCTokenKind token);
bool source_isUnaryOp(OCTokenKind token);

int source_binaryPrecedence(OCTokenKind token);
int source_binaryRightAssoc(OCTokenKind token);

#endif /* orbit_source_tokens_h */
