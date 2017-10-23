//
//  orbit/parser/recursive_descent.c
//  Orbit - Parser
//
//  Created by Amy Parent on 2017-10-23.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdio.h>
#include <stdarg.h>
#include <orbit/console/console.h>
#include <orbit/parser/lexer.h>
#include "recursive_descent.h"

// MARK: - Error printing
// TODO: move to console/diagnostics.h

void compilerError(OCParser* parser, const char* fmt, ...) {
    OASSERT(parser != NULL, "Null instance error");
    if(parser->recovering) { return; }
    parser->recovering = true;
    
    fprintf(stderr, "%s:%llu:%llu: ",
                     parser->lexer.source.path,
                     parser->lexer.currentToken.line,
                     parser->lexer.currentToken.column);
    console_setColor(stderr, CLI_RED);
    fprintf(stderr, "error: ");
    console_setColor(stderr, CLI_RESET);
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fputc('\n', stderr);
    lexer_printLine(stderr, &parser->lexer);
    console_printUnderlines(stderr, parser->lexer.currentToken.column,
                                    parser->lexer.currentToken.displayWidth);
}

void syntaxError(OCParser* parser, OCTokenType type) {
    OASSERT(parser != NULL, "Null instance error");
    if(parser->recovering) { return; }
    parser->recovering = true;
    
    OCToken tok  = current(parser);
    fprintf(stderr, "%s:%llu:%llu: ", parser->lexer.source.path, tok.line, tok.column);
    console_setColor(stderr, CLI_RED);
    fprintf(stderr, "error: ");
    console_setColor(stderr, CLI_RESET);
    fprintf(stderr, "expected '%s'\n", orbit_tokenString(type));
    lexer_printLine(stderr, &parser->lexer);
    console_printUnderlines(stderr, tok.column, parser->lexer.currentToken.displayWidth);
}

// MARK: - RD Basics

bool have(OCParser* parser, OCTokenType type) {
    return parser->lexer.currentToken.type == type;
}

bool match(OCParser* parser, OCTokenType type) {
    if(have(parser, type)) {
        lexer_nextToken(&parser->lexer);
        return true;
    }
    return false;
}

// MARK: - utility functions, mainly used to avoid typing long [have()] lists
bool haveBinaryOp(OCParser* parser) {
    return orbit_isBinaryOp(parser->lexer.currentToken.type);
}

bool haveUnaryOp(OCParser* parser) {
    return orbit_isUnaryOp(parser->lexer.currentToken.type);
}

bool haveConditional(OCParser* parser) {
    return have(parser, TOKEN_IF)
        || have(parser, TOKEN_FOR)
        || have(parser, TOKEN_WHILE);
}

bool haveTerm(OCParser* parser) {
    return haveUnaryOp(parser)
        || have(parser, TOKEN_LPAREN)
        || have(parser, TOKEN_IDENTIFIER)
        || have(parser, TOKEN_STRING_LITERAL)
        || have(parser, TOKEN_INTEGER_LITERAL)
        || have(parser, TOKEN_FLOAT_LITERAL);
}

bool havePrimitiveType(OCParser* parser) {
    return have(parser, TOKEN_NUMBER)
        || have(parser, TOKEN_BOOL)
        || have(parser, TOKEN_BOOL)
        || have(parser, TOKEN_STRING)
        || have(parser, TOKEN_NIL)
        || have(parser, TOKEN_VOID)
        || have(parser, TOKEN_ANY);
}

bool haveType(OCParser* parser) {
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

bool implicitTerminator(OCParser* parser) {
    return parser->lexer.currentToken.startOfLine
        || have(parser, TOKEN_EOF)
        || have(parser, TOKEN_RBRACE);
}

bool expectTerminator(OCParser* parser) {
    if(parser->recovering) {
        while(!have(parser, TOKEN_SEMICOLON) && !have(parser, TOKEN_EOF) && !implicitTerminator(parser)) {
            lexer_nextToken(&parser->lexer);
        }
        if(have(parser, TOKEN_EOF)) {
            return false;
        }
        parser->recovering = false;
        return have(parser, TOKEN_SEMICOLON) ? match(parser, TOKEN_SEMICOLON) : true;
    } else {
        if(match(parser, TOKEN_SEMICOLON) || implicitTerminator(parser)) {
            return true;
        }
        syntaxError(parser, TOKEN_SEMICOLON);
        return false;
    }
}

bool expect(OCParser* parser, OCTokenType type) {
    if(parser->recovering) {
        while(!have(parser, type) && !have(parser, TOKEN_EOF)) {
            lexer_nextToken(&parser->lexer);
        }
        if(have(parser, TOKEN_EOF)) {
            return false;
        }
        parser->recovering = false;
        return match(parser, type);
    } else {
        if(match(parser, type)) { return true; }
        syntaxError(parser, type);
        return false;
    }
}