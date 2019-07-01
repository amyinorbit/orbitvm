//===--------------------------------------------------------------------------------------------===
// orbit/parser/recursive_descent.c
// This source is part of Orbit - Parser
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <orbit/ast/diag.h>

#include "parser_private.h"
#include "recursive_descent.h"

// MARK: - RD Basics

void simpleParseError(OCParser* parser, const char* message) {
    OrbitToken token = current(parser);
    OrbitDiagID id = orbit_diagError(
        &parser->context->diagnostics, current(parser).sourceLoc,
        message, 0
    );
    orbit_diagAddSourceRange(id, orbit_srangeFromLength(token.sourceLoc, token.length));
}

bool have(OCParser* parser, OrbitTokenKind kind) {
    return current(parser).kind == kind;
}

bool match(OCParser* parser, OrbitTokenKind kind) {
    if(have(parser, kind)) {
        orbit_parserNextToken(parser);
        return true;
    }
    return false;
}

// MARK: - utility functions, mainly used to avoid typing long [have()] lists
bool haveBinaryOp(OCParser* parser) {
    return orbit_tokenIsBinaryOp(current(parser).kind);
}

bool haveUnaryOp(OCParser* parser) {
    return orbit_tokenIsUnaryOp(current(parser).kind);
}

bool haveConditional(OCParser* parser) {
    return have(parser, ORBIT_TOK_IF)
        || have(parser, ORBIT_TOK_FOR)
        || have(parser, ORBIT_TOK_WHILE);
}

bool haveTerm(OCParser* parser) {
    return haveUnaryOp(parser)
        || have(parser, ORBIT_TOK_RSLASH)
        || have(parser, ORBIT_TOK_LPAREN)
        || have(parser, ORBIT_TOK_IDENTIFIER)
        || have(parser, ORBIT_TOK_STRING_LITERAL)
        || have(parser, ORBIT_TOK_INTEGER_LITERAL)
        || have(parser, ORBIT_TOK_FLOAT_LITERAL);
}

bool havePrimitiveType(OCParser* parser) {
    return have(parser, ORBIT_TOK_INT)
        || have(parser, ORBIT_TOK_FLOAT)
        || have(parser, ORBIT_TOK_BOOL)
        || have(parser, ORBIT_TOK_STRING)
        || have(parser, ORBIT_TOK_NIL)
        || have(parser, ORBIT_TOK_VOID)
        || have(parser, ORBIT_TOK_ANY);
}

bool haveType(OCParser* parser) {
    return havePrimitiveType(parser)
        || have(parser, ORBIT_TOK_MAYBE)
        || have(parser, ORBIT_TOK_LPAREN)
        || have(parser, ORBIT_TOK_ARRAY)
        || have(parser, ORBIT_TOK_MAP)
        || have(parser, ORBIT_TOK_IDENTIFIER);
}

// Few functions to allow optional semicolons, swift-style.
// https://stackoverflow.com/questions/17646002
//

bool implicitTerminator(OCParser* parser) {
    return parser->currentToken.isStartOfLine
        || have(parser, ORBIT_TOK_EOF)
        || have(parser, ORBIT_TOK_RBRACE);
}

bool expectTerminator(OCParser* parser) {
    if(parser->recovering) {
        while(!have(parser, ORBIT_TOK_SEMICOLON) && !have(parser, ORBIT_TOK_EOF) && !implicitTerminator(parser)) {
            orbit_parserNextToken(parser);
        }
        if(have(parser, ORBIT_TOK_EOF)) {
            return false;
        }
        parser->recovering = false;
        return have(parser, ORBIT_TOK_SEMICOLON) ? match(parser, ORBIT_TOK_SEMICOLON) : true;
    } else {
        if(match(parser, ORBIT_TOK_SEMICOLON) || implicitTerminator(parser)) {
            return true;
        }
        orbit_diagError(
            diag(parser), current(parser).sourceLoc,
            "consecutive statements on a line must be separated by ';'", 0
        );
        return true;
    }
}

bool expect(OCParser* parser, OrbitTokenKind kind) {
    if(parser->recovering) {
        while(!have(parser, kind) && !have(parser, ORBIT_TOK_EOF)) {
            orbit_parserNextToken(parser);
        }
        if(have(parser, ORBIT_TOK_EOF)) { return false; }
        
        parser->recovering = false;
        return match(parser, kind);
    } else {
        if(match(parser, kind)) { return true; }
        OrbitToken token = current(parser);
        OrbitDiagID id = orbit_diagError(
            diag(parser), current(parser).sourceLoc,
            "$0 found while $1 was expected", 2,
            ORBIT_DIAG_CSTRING(orbit_tokenString(current(parser).kind)),
            ORBIT_DIAG_CSTRING(orbit_tokenString(kind))
        );
        orbit_diagAddSourceRange(id, orbit_srangeFromLength(token.sourceLoc, token.length));
        
        parser->recovering = true;
        return false;
    }
}
