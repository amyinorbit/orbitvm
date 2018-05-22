//===--------------------------------------------------------------------------------------------===
// orbit/csupport/tokens.h
// This source is part of Orbit - Compiler Support
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_csupport_tokens_h
#define orbit_csupport_tokens_h

#include <stdint.h>
#include <stdbool.h>
#include <orbit/csupport/string.h>
#include <orbit/csupport/source.h>

typedef struct _OrbitToken OrbitToken;

typedef enum  {
    // Bracket types
    // (){}[]
    ORBIT_TOK_LPAREN,
    ORBIT_TOK_RPAREN,
    ORBIT_TOK_LBRACE,
    ORBIT_TOK_RBRACE,
    ORBIT_TOK_LBRACKET,
    ORBIT_TOK_RBRACKET,
    
    // Orbit Tokens
    // + - / * % ^ & | < > = >= <= == += ~ >> << && || != ! ?
    ORBIT_TOK_PLUS,
    ORBIT_TOK_MINUS,
    ORBIT_TOK_SLASH,
    ORBIT_TOK_STAR,
    ORBIT_TOK_STARSTAR,
    ORBIT_TOK_PERCENT,
    ORBIT_TOK_CARET,
    ORBIT_TOK_TILDE,
    ORBIT_TOK_AMP,
    ORBIT_TOK_PIPE,
    ORBIT_TOK_BANG,
    ORBIT_TOK_QUESTION,
    ORBIT_TOK_LT,
    ORBIT_TOK_GT,
    ORBIT_TOK_EQUALS,
    ORBIT_TOK_LTEQ,
    ORBIT_TOK_GTEQ,
    ORBIT_TOK_EQEQ,
    ORBIT_TOK_PLUSEQ,
    ORBIT_TOK_MINUSEQ,
    ORBIT_TOK_STAREQ,
    ORBIT_TOK_SLASHEQ,
    ORBIT_TOK_BANGEQ,
    ORBIT_TOK_LTLT,
    ORBIT_TOK_GTGT,
    ORBIT_TOK_AMPAMP,
    ORBIT_TOK_PIPEPIPE,
    
    ORBIT_TOK_SEMICOLON,
    ORBIT_TOK_NEWLINE,
    
    // Punctuation:
    // :,.
    ORBIT_TOK_COLON,
    ORBIT_TOK_COMMA,
    ORBIT_TOK_DOT,
    ORBIT_TOK_ARROW,
    
    // Language objects
    ORBIT_TOK_INTEGER_LITERAL,
    ORBIT_TOK_FLOAT_LITERAL,
    //ORBIT_TOK_EXP_LITERAL, // later
    ORBIT_TOK_STRING_LITERAL,
    ORBIT_TOK_IDENTIFIER,
    
    // Language keywords
    ORBIT_TOK_FUN,
    ORBIT_TOK_VAR,
    ORBIT_TOK_CONST,
    ORBIT_TOK_MAYBE,
    ORBIT_TOK_TYPE,
    ORBIT_TOK_RETURN,
    ORBIT_TOK_FOR,
    ORBIT_TOK_WHILE,
    ORBIT_TOK_BREAK,
    ORBIT_TOK_CONTINUE,
    ORBIT_TOK_IN,
    ORBIT_TOK_IF,
    ORBIT_TOK_ELSE,
    ORBIT_TOK_INIT,
    ORBIT_TOK_FAIL,
    ORBIT_TOK_RANGE,
    
    // Types
    ORBIT_TOK_NUMBER,
    ORBIT_TOK_BOOL,
    ORBIT_TOK_STRING,
    ORBIT_TOK_NIL,
    ORBIT_TOK_VOID,
    ORBIT_TOK_ARRAY,
    ORBIT_TOK_MAP,
    ORBIT_TOK_ANY,
    
    ORBIT_TOK_EOF,
    ORBIT_TOK_INVALID,
    
} OrbitTokenKind;


struct _OrbitToken {
    OrbitTokenKind  kind;
    const OrbitSource* source;
    OrbitSLoc       sourceLoc;
    OCStringID      parsedStringLiteral;
    uint32_t        length:31;
    uint32_t        isStartOfLine:1;
};

const char* orbit_tokenName(OrbitTokenKind token);
const char* orbit_tokenString(OrbitTokenKind token);

bool orbit_tokenEquals(OrbitToken* a, OrbitToken* b);

bool orbit_tokenIsBinaryOp(OrbitTokenKind token);
bool orbit_tokenIsUnaryOp(OrbitTokenKind token);

int orbit_tokenBinaryPrecedence(OrbitTokenKind token);
int orbit_tokenBinaryRightAssoc(OrbitTokenKind token);

#endif /* orbit_csupport_tokens_h */
