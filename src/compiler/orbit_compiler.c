//
//  orbit_compiler.c
//  OrbitVM
//
//  Created by Amy Parent on 2017-03-01.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <orbit/orbit_utils.h>
#include <orbit/orbit_utf8.h>
#include "orbit_lexer.h"

static char* loadSource(const char* path, uint64_t* length) {
    FILE* f = fopen(path, "r");
    if(!f) {
        *length = 0;
        return NULL; 
    }

    fseek(f, 0, SEEK_END);
    uint64_t len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* source = malloc((len+1) * sizeof(char));
    fread(source, sizeof(char), len, f);
    source[len] = '\0';
    fclose(f);
    
    *length = len;
    return source;
}

typedef enum {
    LEFT = -1,
    RIGHT = 1,
} _Direction;

typedef struct {
    OCTokenType     name;
    int             precedence;
    _Direction      assoc;
} OCOperator;

static OCOperator opTable[] = {
    {TOKEN_STARSTAR,    10,     RIGHT},
    {TOKEN_STAR,        9,      LEFT},
    {TOKEN_SLASH,       9,      LEFT},
    {TOKEN_PERCENT,     9,      LEFT},
    {TOKEN_PLUS,        8,      LEFT},
    {TOKEN_MINUS,       8,      LEFT},
    {TOKEN_LTLT,        7,      LEFT},
    {TOKEN_GTGT,        7,      LEFT},
    {TOKEN_LT,          6,      LEFT},
    {TOKEN_GT,          6,      LEFT},
    {TOKEN_LTEQ,        6,      LEFT},
    {TOKEN_GTEQ,        6,      LEFT},
    {TOKEN_EQEQ,        5,      LEFT},
    {TOKEN_BANGEQ,      5,      LEFT},
    {TOKEN_AMP,         4,      LEFT},
    {TOKEN_CARET,       3,      LEFT},
    {TOKEN_PIPE,        2,      LEFT},
    {TOKEN_AMPAMP,      1,      LEFT},
    {TOKEN_PIPEPIPE,    0,      LEFT},
    {TOKEN_INVALID,     -1,     LEFT},
};

static inline void parseError(const char* msg) {
    fprintf(stderr, "error: %s\n", msg);
}

static inline bool have(OCLexer* lex, OCTokenType type) {
    return lex->currentToken.type == type;
}

static inline void expect(OCLexer* lex, OCTokenType type) {
    if(have(lex, type)) {
        lexer_nextToken(lex);
    } else {
        parseError("unexpected token");
    }
}

static inline int opPrecedence(OCLexer* lex) {
    for(int i = 0; opTable[i].name != TOKEN_INVALID; ++i) {
        if(lex->currentToken.type == opTable[i].name) {
            return opTable[i].precedence;
        }
    }
    return 0;
}

static inline _Direction opAssoc(OCLexer* lex) {
    for(int i = 0; opTable[i].name != TOKEN_INVALID; ++i) {
        if(lex->currentToken.type == opTable[i].name) {
            return opTable[i].assoc;
        }
    }
    return LEFT;
}

static bool isBinOp(OCLexer* lex) {
    for(int i = 0; opTable[i].name != TOKEN_INVALID; ++i) {
        if(lex->currentToken.type == opTable[i].name) { return true; }
    }
    return false;
}

static void recAtom(OCLexer*);
static void recExpression(OCLexer*, int);

static void recExpression(OCLexer* lex, int minPrec) {
    // LHS
    printf("( ");
    recAtom(lex);
    
    for(;;) {
        if(have(lex, TOKEN_EOF) || have(lex, TOKEN_NEWLINE)
            || !isBinOp(lex) || opPrecedence(lex) < minPrec) {
            break;
        }
        
        OCTokenType op = lex->currentToken.type;
        
        printf("op.%d ", op);
        
        int prec = opPrecedence(lex);
        _Direction assoc = opAssoc(lex);
        
        int nextMinPrec = assoc == LEFT ? prec + 1 : prec;
        
        lexer_nextToken(lex);
        // RHS
        recExpression(lex, nextMinPrec);
    }

    printf(") ");
}

static void recAtom(OCLexer* lex) {
    if(have(lex, TOKEN_MINUS)) {
        expect(lex, TOKEN_MINUS);
        printf("- ");
    }
    
    if(have(lex, TOKEN_LPAREN)) {
        expect(lex, TOKEN_LPAREN);
        recExpression(lex, 0);
        expect(lex, TOKEN_RPAREN);
    }
    else if(have(lex, TOKEN_IDENTIFIER)) {
        printf("%.*s ", lex->currentToken.length, lex->currentToken.start);
        expect(lex, TOKEN_IDENTIFIER);
    }
    else if(have(lex, TOKEN_INTEGER_LITERAL)) {
        printf("%.*s ", lex->currentToken.length, lex->currentToken.start);
        expect(lex, TOKEN_INTEGER_LITERAL);
    }
    else if(have(lex, TOKEN_FLOAT_LITERAL)) {
        printf("%.*s ", lex->currentToken.length, lex->currentToken.start);
        expect(lex, TOKEN_FLOAT_LITERAL);
    }
    else {
        parseError("unexpected token (need term)");
    }
}

int main(int argc, const char** args) {
    
    if(argc != 2) {
        fprintf(stderr, "Invalid format\n");
        return -1;
    }
    
    uint64_t length = 0;
    char* source = loadSource(args[1], &length);
    if(!source) {
        fprintf(stderr, "error opening `%s`\n", args[1]);
        return -1;
    }
    
    OCLexer lex;
    lexer_init(&lex, args[1], source, length);
    
    lexer_nextToken(&lex);
    while(lex.currentToken.type != TOKEN_EOF) {
        recExpression(&lex, 0);
        printf(" // %.*s [%d]\n", lex.currentToken.length, lex.currentToken.start, lex.currentToken.type);
        lexer_nextToken(&lex);
    }
    free(source);
    return 0;
}
