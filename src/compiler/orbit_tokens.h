//
//  orbit_tokens.h
//  OrbitVM
//
//  Created by Amy Parent on 2017-03-01.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_tokens_h
#define orbit_tokens_h

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
    
} OCTokenType;

#endif /* orbit_tokens_h_ */
