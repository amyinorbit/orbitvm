//
//  orbit_parser.c
//  OrbitVM
//
//  Created by Amy Parent on 2017-05-21.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdarg.h>
#include <utils/orbit_assert.h>
#include "orbit_parser.h"
#include "orbit_tokens.h"
#include "orbit_lexer.h"
#include "compiler_utils.h"

typedef struct {
    OCLexer     lexer;
    bool        recovering;
} OCParser;

// MARK: - Basic RD parser utilities

static inline int precedence(OCParser* parser) {
    return orbit_binaryPrecedence(parser->lexer.currentToken.type);
}

static inline bool rightAssoc(OCParser* parser) {
    return orbit_binaryRightAssoc(parser->lexer.currentToken.type);
}

static inline OCToken current(OCParser* parser) {
    return parser->lexer.currentToken;
}

static void compilerError(OCParser* parser, const char* fmt, ...) {
    OASSERT(parser != NULL, "Null instance error");
    if(parser->recovering) { return; }
    parser->recovering = true;
    
    fprintf(stderr, "%s:%llu:%llu: ",
                     parser->lexer.path,
                     parser->lexer.currentToken.line,
                     parser->lexer.currentToken.column);
    orbit_setConsoleColor(stderr, CLI_RED);
    fprintf(stderr, "error: ");
    orbit_setConsoleColor(stderr, CLI_RESET);
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fputc('\n', stderr);
    lexer_printLine(stderr, &parser->lexer);
    orbit_printSquigglies(stderr, parser->lexer.currentToken.column,
                                  parser->lexer.currentToken.displayWidth);
}

static void syntaxError(OCParser* parser, OCTokenType type) {
    OASSERT(parser != NULL, "Null instance error");
    if(parser->recovering) { return; }
    parser->recovering = true;
    
    OCToken tok  = current(parser);
    fprintf(stderr, "%s:%llu:%llu: ", parser->lexer.path, tok.line, tok.column);
    orbit_setConsoleColor(stderr, CLI_RED);
    fprintf(stderr, "error: ");
    orbit_setConsoleColor(stderr, CLI_RESET);
    fprintf(stderr, "expected '%s'\n", orbit_tokenString(type));
    lexer_printLine(stderr, &parser->lexer);
    orbit_printSquigglies(stderr, tok.column, parser->lexer.currentToken.displayWidth);
}

// MARK: - RD Basics

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

// MARK: - utility functions, mainly used to avoid typing long [have()] lists
static inline bool haveBinaryOp(OCParser* parser) {
    return orbit_isBinaryOp(parser->lexer.currentToken.type);
}

static inline bool haveUnaryOp(OCParser* parser) {
    return orbit_isUnaryOp(parser->lexer.currentToken.type);
}

static inline bool haveConditional(OCParser* parser) {
    return have(parser, TOKEN_IF)
        || have(parser, TOKEN_FOR) 
        || have(parser, TOKEN_WHILE);
}

static inline bool haveTerm(OCParser* parser) {
    return haveUnaryOp(parser) 
        || have(parser, TOKEN_LPAREN)
        || have(parser, TOKEN_IDENTIFIER)
        || have(parser, TOKEN_STRING_LITERAL)
        || have(parser, TOKEN_INTEGER_LITERAL)
        || have(parser, TOKEN_FLOAT_LITERAL);
}

static inline bool havePrimitiveType(OCParser* parser) {
    return have(parser, TOKEN_NUMBER)
        || have(parser, TOKEN_BOOL)
        || have(parser, TOKEN_BOOL)
        || have(parser, TOKEN_STRING)
        || have(parser, TOKEN_NIL)
        || have(parser, TOKEN_VOID)
        || have(parser, TOKEN_ANY);
}

static inline bool haveType(OCParser* parser) {
    return havePrimitiveType(parser)
        || have(parser, TOKEN_MAYBE)
        || have(parser, TOKEN_LPAREN)
        || have(parser, TOKEN_ARRAY)
        || have(parser, TOKEN_MAP)
        || have(parser, TOKEN_IDENTIFIER);
}

// Few functions to allow optional semicolons, swift-style.
// https://stackoverflow.com/questions/17646002
//

static inline bool implicitTerminator(OCParser* parser) {
    return parser->lexer.currentToken.startOfLine
        || have(parser, TOKEN_EOF)
        || have(parser, TOKEN_RBRACE);
}

static bool expectTerminator(OCParser* parser) {
    if(parser->recovering) {
        while(!have(parser, TOKEN_SEMICOLON) || implicitTerminator(parser)) {
            if(have(parser, TOKEN_EOF)) { break; }
            lexer_nextToken(&parser->lexer);
        }
        parser->recovering = false;
        match(parser, TOKEN_SEMICOLON);
        return true;
    } else {
        if(match(parser, TOKEN_SEMICOLON) || implicitTerminator(parser)) {
            return true;
        }
        syntaxError(parser, TOKEN_SEMICOLON);
        return false;
    }
}

static bool expect(OCParser* parser, OCTokenType type) {
    if(parser->recovering) {
        while(!have(parser, type)) {
            if(have(parser, TOKEN_EOF)) { break; }
            lexer_nextToken(&parser->lexer);
        }
        parser->recovering = false;
        return match(parser, type);
    } else {
        if(match(parser, type)) { return true; }
        syntaxError(parser, type);
        return false;
    }
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
static void recFlowStatement(OCParser*);
static void recReturnStatement(OCParser*);

static void recExpression(OCParser*, int);
static void recTerm(OCParser*);
static void recName(OCParser*);
//static void recSuffix(OCParser*);
static void recSubscript(OCParser*);
static void recFieldAccess(OCParser*);
static void recFuncCall(OCParser*);
static void recExprList(OCParser*);

static void recType(OCParser*);
static void recFuncType(OCParser*);
static void recTypename(OCParser*);
static void recPrimitive(OCParser*);
static void recArrayType(OCParser*);
static void recMapType(OCParser*);

//static void recNumberLiteral(OCParser*);
//static void recStringLiteral(OCParser*);

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
        expectTerminator(parser);
    }
    expect(parser, TOKEN_EOF);
}

static void recBlock(OCParser* parser) {
    expect(parser, TOKEN_LBRACE);
    for(;;) {
        if(have(parser, TOKEN_VAR))
            recVarDecl(parser);
        else if(haveTerm(parser)
            || haveConditional(parser)
            || have(parser, TOKEN_RETURN)
            || have(parser, TOKEN_BREAK)
            || have(parser, TOKEN_CONTINUE))
            recStatement(parser);
        else
            break;
        expectTerminator(parser);
        
    }
    expect(parser, TOKEN_RBRACE);
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
    
    recType(parser);
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
    if(haveConditional(parser)) {
        recConditional(parser);
    }
    else if(haveTerm(parser)) {
        recExpression(parser, 0);
    }
    else if(have(parser, TOKEN_RETURN)) {
        recReturnStatement(parser);
    }
    else if(have(parser, TOKEN_BREAK) || have(parser, TOKEN_CONTINUE)) {
        recFlowStatement(parser);
    }
    else {
        compilerError(parser, "expected a statement");
    }
}

static void recConditional(OCParser* parser) {
    if(have(parser, TOKEN_IF))
        recIfStatement(parser);
    else if(have(parser, TOKEN_WHILE))
        recWhileLoop(parser);
    else if(have(parser, TOKEN_FOR))
        recForLoop(parser);
    else
        compilerError(parser, "expected an if statement or a loop");
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

static void recFlowStatement(OCParser* parser) {
    if(have(parser, TOKEN_BREAK))
        expect(parser, TOKEN_BREAK);
    else if(have(parser, TOKEN_CONTINUE))
        expect(parser, TOKEN_CONTINUE);
    else
        compilerError(parser, "expected break or continue");
        
}

static void recReturnStatement(OCParser* parser) {
    expect(parser, TOKEN_RETURN);
    if(haveTerm(parser)) {
        recExpression(parser, 0);
    }
}

static void recWhileLoop(OCParser* parser) {
    expect(parser, TOKEN_WHILE);
    recExpression(parser, 0);
    recBlock(parser);
}

static void recForLoop(OCParser* parser) {
    expect(parser, TOKEN_FOR);
    expect(parser, TOKEN_IDENTIFIER);
    expect(parser, TOKEN_IN);
    recExpression(parser, 0);
    recBlock(parser);
}

// We don't embed precedence in the grammar - instead any expression
// is parsed using precedence-climbing
// http://eli.thegreenplace.net/2012/08/02/parsing-expressions-by-precedence-climbing
static void recExpression(OCParser* parser, int minPrec) {
    recTerm(parser);
    for(;;) {
        if(!haveBinaryOp(parser) || precedence(parser) < minPrec) {
            break;
        }
        //OCToken operator = current(parser);
        int prec = precedence(parser);
        bool right = rightAssoc(parser);
        
        int nextMinPrec = right ? prec : prec + 1;
        lexer_nextToken(&parser->lexer);
        recExpression(parser, nextMinPrec);
    }
}

static void recTerm(OCParser* parser) {
    if(haveUnaryOp(parser)) {
        lexer_nextToken(&parser->lexer);
    }
    
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
        else if(have(parser, TOKEN_DOT))
            recFieldAccess(parser);
        else if(have(parser, TOKEN_LPAREN))
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
    expect(parser, TOKEN_IDENTIFIER);
}

static void recFuncCall(OCParser* parser) {
    expect(parser, TOKEN_LPAREN);
    if(haveTerm(parser)) {
        recExprList(parser);
    }
    expect(parser, TOKEN_RPAREN);
}

static void recExprList(OCParser* parser) {
    recExpression(parser, 0);
    while(match(parser, TOKEN_COMMA)) {
        recExpression(parser, 0);
    }
}

static void recType(OCParser* parser) {
    if(match(parser, TOKEN_MAYBE)) {
    }
    recTypename(parser);
}


static void recTypename(OCParser* parser) {
    if(havePrimitiveType(parser)) {
       recPrimitive(parser);
   }
   else if(have(parser, TOKEN_LPAREN)) {
       recFuncType(parser);
   }
   else if(have(parser, TOKEN_ARRAY)) {
       recArrayType(parser);
   }
   else if(have(parser, TOKEN_MAP)) {
       recMapType(parser);
   }
   else if(have(parser, TOKEN_IDENTIFIER)) {
       // TODO: user type
       expect(parser, TOKEN_IDENTIFIER);
   }
   else {
       compilerError(parser, "expected a type name");
   }
}

static void recFuncType(OCParser* parser) {
    expect(parser, TOKEN_LPAREN);
    
    if(haveType(parser)) {
        recType(parser);
        while(match(parser, TOKEN_COMMA)) {
            recType(parser);
        }
    }
    expect(parser, TOKEN_RPAREN);
    expect(parser, TOKEN_ARROW);
    recType(parser);
}

static void recPrimitive(OCParser* parser) {
    if(have(parser, TOKEN_NUMBER))
        expect(parser, TOKEN_NUMBER);
    else if(have(parser, TOKEN_BOOL))
        expect(parser, TOKEN_BOOL);
    else if(have(parser, TOKEN_STRING))
        expect(parser, TOKEN_STRING);
    else if(have(parser, TOKEN_NIL))
        expect(parser, TOKEN_NIL);
    else if(have(parser, TOKEN_VOID))
        expect(parser, TOKEN_VOID);
    else if(have(parser, TOKEN_ANY))
        expect(parser, TOKEN_ANY);
    else
        compilerError(parser, "expected a primitive type");
}

static void recArrayType(OCParser* parser) {
    expect(parser, TOKEN_ARRAY);
    expect(parser, TOKEN_LBRACKET);
    recType(parser);
    expect(parser, TOKEN_RBRACKET);
}

static void recMapType(OCParser* parser) {
    expect(parser, TOKEN_MAP);
    expect(parser, TOKEN_LBRACKET);
    recPrimitive(parser);
    expect(parser, TOKEN_COLON);
    recType(parser);
    expect(parser, TOKEN_RBRACKET);
}

// static void recNumberLiteral(OCParser* parser) {
//     if(have(parser, TOKEN_INTEGER_LITERAL))
//         expect(parser, TOKEN_INTEGER_LITERAL);
//     if(have(parser, TOKEN_FLOAT_LITERAL))
//         expect(parser, TOKEN_FLOAT_LITERAL);
//     else
//         compilerError(parser, "expected a number constant");
//
// }
//
// static void recStringLiteral(OCParser* parser) {
//     expect(parser, TOKEN_STRING_LITERAL);
// }

void orbit_dumpTokens(const char* sourcePath, const char* source, uint64_t length) {
    OCLexer lex;
    lexer_init(&lex, sourcePath, source, length);
    
    lexer_nextToken(&lex);
    while(lex.currentToken.type != TOKEN_EOF) {
        OCToken tok = lex.currentToken;
        printf("%20s\t'%.*s'", orbit_tokenName(tok.type), (int)tok.length, tok.start);
        if(tok.startOfLine) {
            printf(" [line start]");
        }
        putchar('\n');
        lexer_nextToken(&lex);
    }
}

bool orbit_compile(const char* sourcePath, const char* source, uint64_t length) {
    
    OCParser parser;
    parser.recovering = false;
    lexer_init(&parser.lexer, sourcePath, source, length);
    
    lexer_nextToken(&parser.lexer);
    recProgram(&parser);
    return true;
}

