//
//  orbit_parser.c
//  OrbitVM
//
//  Created by Amy Parent on 2017-05-21.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdarg.h>
#include <orbit/orbit_utils.h>
#include "orbit_parser.h"
#include "orbit_tokens.h"
#include "orbit_lexer.h"
#include "compiler_utils.h"

typedef struct {
    OCLexer     lexer;
    bool        recovering;
    OrbitVM*    vm;
} OCParser;

// MARK: - Basic RD parser utilities

static void compilerError(OCParser* parser, const char* fmt, ...) {
    OASSERT(parser != NULL, "Null instance error");
    
    fprintf(stderr, "%s:%llu:%llu: error: ",
                     parser->lexer.path,
                     parser->lexer.currentToken.line,
                     parser->lexer.currentToken.column);
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "\n");
    lexer_printLine(stderr, &parser->lexer);
    fprintf(stderr, "\n");
}

static inline OCToken current(OCParser* parser) {
    return parser->lexer.currentToken;
}

static inline bool have(OCParser* parser, OCTokenType type) {
    return parser->lexer.currentToken.type == type;
}

static inline bool match(OCParser* parser, OCTokenType type) {
    if(have(parser, type)) {
        lexer_nextToken(&parser->lexer);
        return true;
    }
    return false;
}

static bool expect(OCParser* parser, OCTokenType type) {
    if(match(parser, type)) { return true; }
    compilerError(parser, "found '%s' (expected '%s')",
                  orbit_tokenString(parser->lexer.currentToken.type),
                  orbit_tokenString(type));
    return false;
}



// The Big List: all of our recognisers. Easier to forward-declare to avoid
// shuffling everything every time we had a recogniser.

//static void recDecl(OCParser*);
static void recBlock(OCParser*);
static void recTypeDecl(OCParser*);
static void recVarDecl(OCParser*);
static void recFuncDecl(OCParser*);
static void recParameters(OCParser*);

static void recStatement(OCParser*);
static void recConditional(OCParser*);
static void recIfStatement(OCParser*);
static void recWhileLoop(OCParser*);
static void recForLoop(OCParser*);

static void recExpression(OCParser*, int);
static void recTerm(OCParser*);
static void recName(OCParser*);
static void recSuffix(OCParser*);
static void recSubscript(OCParser*);
static void recFieldAccess(OCParser*);
static void recFuncCall(OCParser*);
static void recExprList(OCParser*);

static void recType(OCParser*);
static void recTypename(OCParser*);
static void recPrimitive(OCParser*);
static void recArrayType(OCParser*);
static void recMapType(OCParser*);

static void recNumberLiteral(OCParser*);
static void recStringLiteral(OCParser*);

// MARK: - Implementation

static void recProgram(OCParser* parser) {
    for(;;) {
        if(have(parser, TOKEN_VAR))
            recVarDecl(parser);
        else if(have(parser, TOKEN_FUN))
            recFuncDecl(parser);
        else if(have(parser, TOKEN_TYPE))
            recTypeDecl(parser);
        else
            break;
    }
    expect(parser, TOKEN_EOF);
}

static void recBlock(OCParser* parser) {
    
}

static void recTypeDecl(OCParser* parser) {
    expect(parser, TOKEN_TYPE);
    expect(parser, TOKEN_IDENTIFIER);
    expect(parser, TOKEN_LBRACE);
    
    do {
        recVarDecl(parser);
    } while(have(parser, TOKEN_VAR));
    
    expect(parser, TOKEN_RBRACE);
}

// 'var' identifier ((':', type) | ((':', type)? '=' expression))
static void recVarDecl(OCParser* parser) {
    expect(parser, TOKEN_VAR);
    expect(parser, TOKEN_IDENTIFIER);
    
    if(match(parser, TOKEN_COLON)) {
        recType(parser);
    }
    
    if(match(parser, TOKEN_EQUALS)) {
        recExpression(parser, 0);
    }
}

// func-proto '{' block '}'
// 'func' identifier parameters '->' type
static void recFuncDecl(OCParser* parser) {
    expect(parser, TOKEN_FUN);
    expect(parser, TOKEN_IDENTIFIER);
    expect(parser, TOKEN_LPAREN);
    
    if(have(parser, TOKEN_IDENTIFIER)) {
        recParameters(parser);
    }
    
    expect(parser, TOKEN_RPAREN);
    expect(parser, TOKEN_ARROW);
    expect(parser, TOKEN_TYPE);
    
    recBlock(parser);
}


static void recParameters(OCParser* parser) {
    
    do {
        expect(parser, TOKEN_IDENTIFIER);
        expect(parser, TOKEN_COLON);
        recType(parser);
    } while(match(parser, TOKEN_COMMA));
}

static void recStatement(OCParser* parser) {
    
}

static void recConditional(OCParser* parser) {
    
}

static void recIfStatement(OCParser* parser) {
    expect(parser, TOKEN_IF);
    recExpression(parser, 0);
    recBlock(parser);
    
    if(match(parser, TOKEN_ELSE)) {
        if(have(parser, TOKEN_LBRACE))
            recBlock(parser);
        else if(have(parser, TOKEN_IF))
            recIfStatement(parser);
        else
            compilerError(parser, "expected block or if statement");
    }
}

static void recWhileLoop(OCParser* parser) {
    expect(parser, TOKEN_WHILE);
    recExpression(parser, 0);
    recBlock(parser);
}

static void recForStatement(OCParser* parser) {
    expect(parser, TOKEN_FOR);
    expect(parser, TOKEN_IDENTIFIER);
    expect(parser, TOKEN_COLON);
    recExpression(parser, 0);
    recBlock(parser);
}

static void recExpression(OCParser* parser, int minPrec) {
    
    recTerm(parser);
    for(;;) {
        // TODO: implement precedence climbing
    }
    /*
    // LHS
    //printf("( ");
    recAtom(lex);
    
    for(;;) {
        if(have(lex, TOKEN_EOF) || have(lex, TOKEN_NEWLINE)
            || !isBinOp(lex) || opPrecedence(lex) < minPrec) {
            break;
        }
        
        OCTokenType op = lex->currentToken.type;
        
        int prec = opPrecedence(lex);
        _Direction assoc = opAssoc(lex);
        
        int nextMinPrec = assoc == LEFT ? prec + 1 : prec;
        
        lexer_nextToken(lex);
        // RHS
        recExpression(lex, nextMinPrec);
        printf("EXEC op.%d\n", op);
    }
    */
}

static void recTerm(OCParser* parser) {
    // TODO: match unary operator
    if(match(parser, TOKEN_LPAREN)) {
        recExpression(parser, 0);
        expect(parser, TOKEN_RPAREN);
    }
    else if(have(parser, TOKEN_IDENTIFIER)) {
        recName(parser);
    }
    else if(have(parser, TOKEN_STRING_LITERAL)) {
        expect(parser, TOKEN_STRING_LITERAL);
    }
    else if(have(parser, TOKEN_INTEGER_LITERAL)) {
        expect(parser, TOKEN_INTEGER_LITERAL);
    }
    else if(have(parser, TOKEN_FLOAT_LITERAL)) {
        expect(parser, TOKEN_FLOAT_LITERAL);
    }
    else {
        compilerError(parser, "expected an expression term");
    }
}

static void recName(OCParser* parser) {
    expect(parser, TOKEN_IDENTIFIER);
    for(;;) {
        if(have(parser, TOKEN_LBRACKET))
            recSubscript(parser);
        if(have(parser, TOKEN_DOT))
            recFieldAccess(parser);
        if(have(parser, TOKEN_LPAREN))
            recFuncCall(parser);
        else
            break;
    }
}

static void recSubscript(OCParser* parser) {
    expect(parser, TOKEN_LBRACKET);
    recExpression(parser, 0);
    expect(parser, TOKEN_RBRACKET);
}

static void recFieldAccess(OCParser* parser) {
    expect(parser, TOKEN_DOT);
    recName(parser);
}

static void recFuncCall(OCParser* parser) {
    expect(parser, TOKEN_LBRACKET);
    recExprList(parser); // TODO: surround with proper director list
    expect(parser, TOKEN_RBRACKET);
}

static void recExprList(OCParser* parser) {
    recExpression(parser, 0);
    while(match(parser, TOKEN_COMMA)) {
        recExpression(parser, 0);
    }
}

static void recType(OCParser* parser) {
    
}

static void recTypename(OCParser* parser) {
    
}

static void recPrimitive(OCParser* parser) {
    
}

static void recArrayType(OCParser* parser) {
    
}

static void recMapType(OCParser* parser) {
    
}

static void recNumberLiteral(OCParser* parser) {
    
}

static void recStringLiteral(OCParser* parser) {
    
}

bool orbit_compile(OrbitVM* vm, const char* sourcePath, const char* source, uint64_t length) {
    
    OCParser parser;
    parser.vm = vm;
    parser.recovering = false;
    lexer_init(&parser.lexer, sourcePath, source, length);
    
    lexer_nextToken(&parser.lexer);
    while(!have(&parser, TOKEN_EOF)) {
        OCToken tok = current(&parser);
        printf("%20s\t'%.*s'\n", orbit_tokenName(tok.type), (int)tok.length, tok.start);
        lexer_nextToken(&parser.lexer);
    }
    return true;
}

