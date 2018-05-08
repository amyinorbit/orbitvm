//===--------------------------------------------------------------------------------------------===
// orbit/parser/recursive_descent.c
// This source is part of Orbit - Parser
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
//#include <orbit/csupport/console.h>
#include <orbit/csupport/diag.h>
#include <orbit/parser/lexer.h>
#include <orbit/utils/assert.h>
#include "recursive_descent.h"

// MARK: - Error printing
// TODO: move to console/diagnostics.h

void compilerError(OCParser* parser, const char* fmt, ...) {
    // OASSERT(parser != NULL, "Null instance error");
    // if(parser->recovering) { return; }
    // parser->recovering = true;
    // 
    // OCToken tok  = current(parser);
    // 
    // console_setColor(stderr, CLI_BOLD);
    // console_setColor(stderr, CLI_BLUE);
    // fputs("----- ", stderr);
    // console_setColor(stderr, CLI_RED);
    // fputs("SYNTAX ERROR", stderr);
    // console_setColor(stderr, CLI_BLUE);
    // 
    // fprintf(stderr, " in %s\n", parser->lexer.source->path);
    // 
    // console_setColor(stderr, CLI_RESET);
    // console_printTokenLine(stderr, current(parser));
    // console_printUnderlines(stderr, tok, CLI_RED);
    // 
    // console_setColor(stderr, CLI_BOLD);
    // va_list va;
    // va_start(va, fmt);
    // vfprintf(stderr, fmt, va);
    // va_end(va);
    // fputc('\n', stderr);
    OrbitDiagID id = orbit_diagNew(
        &orbit_defaultDiagManager,
        ORBIT_DIAGLEVEL_ERROR,
        fmt
    );
    orbit_diagAddSourceLoc(id, current(parser).sourceLoc);
}

void syntaxError(OCParser* parser, OCTokenKind kind) {
    OrbitDiagID id = orbit_diagNew(
        &orbit_defaultDiagManager,
        ORBIT_DIAGLEVEL_ERROR,
        "$0 was expected, but $1 was found instead"
    );
    orbit_diagAddParam(id, ORBIT_DIAG_CSTRING(source_tokenString(kind)));
    orbit_diagAddParam(id, ORBIT_DIAG_CSTRING(source_tokenString(current(parser).kind)));
    orbit_diagAddSourceLoc(id, current(parser).sourceLoc);
}

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
        syntaxError(parser, TOKEN_SEMICOLON);
        return false;
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
        syntaxError(parser, kind);
        return false;
    }
}
