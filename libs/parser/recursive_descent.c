//===--------------------------------------------------------------------------------------------===
// orbit/parser/recursive_descent.c
// This source is part of Orbit - Parser
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <orbit/ast/diag.h>
#include <orbit/parser/lexer.h>
#include "recursive_descent.h"

// MARK: - RD Basics

bool have(OCParser* parser, OCTokenKind kind) {
    return current(parser).kind == kind;
}

bool match(OCParser* parser, OCTokenKind kind) {
    if(have(parser, kind)) {
        lexer_nextToken(&parser->lexer);
        return true;
    }
    return false;
}

// MARK: - utility functions, mainly used to avoid typing long [have()] lists
bool haveBinaryOp(OCParser* parser) {
    return source_isBinaryOp(current(parser).kind);
}

bool haveUnaryOp(OCParser* parser) {
    return source_isUnaryOp(current(parser).kind);
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
    return parser->lexer.currentToken.isStartOfLine
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
        orbit_diagEmitError(
            current(parser).sourceLoc,
            "consecutive statements on a line must be separated by ';'", 0
        );
        return true;
    }
}

bool expect(OCParser* parser, OCTokenKind kind) {
    if(parser->recovering) {
        while(!have(parser, kind) && !have(parser, TOKEN_EOF)) {
            lexer_nextToken(&parser->lexer);
        }
        if(have(parser, TOKEN_EOF)) {
            return false;
        }
        parser->recovering = false;
        return match(parser, kind);
    } else {
        if(match(parser, kind)) { return true; }
        orbit_diagEmitError(
            current(parser).sourceLoc, "$0 found while $1 was expected", 2, ORBIT_DIAG_CSTRING(source_tokenString(current(parser).kind)),
            ORBIT_DIAG_CSTRING(source_tokenString(kind))
        );
        return false;
    }
}
