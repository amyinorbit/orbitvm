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
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_STARSTAR,
    TOKEN_PERCENT,
    TOKEN_BANG,
    TOKEN_BANGEQ,
    TOKEN_QUESTION,
    TOKEN_AND,
    TOKEN_AMPERSAND,
    TOKEN_OR,
    TOKEN_PIPE,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_EQ,
    TOKEN_EQEQ,
    TOKEN_LTEQ,
    TOKEN_GTEQ,
    
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    
    // Language objects
    TOKEN_NUMLIT,
    TOKEN_STRINGLIT,
    TOKEN_IDENT,
    
    // Types
    TOKEN_NUM,
    TOKEN_BOOL,
    TOKEN_STRING,
    TOKEN_NIL,
    
} OCTokenType;

#endif /* orbit_tokens_h_ */
