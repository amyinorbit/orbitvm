//===--------------------------------------------------------------------------------------------===
// orbit/parser/recursive_descent.h
// This source is part of Orbit - Parser
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_parser_recdescent_h
#define orbit_parser_recdescent_h

//#include <orbit/parser/compiler_utils.h>
#include <orbit/csupport/tokens.h>
#include <orbit/parser/lexer.h>

typedef struct {
    OCLexer     lexer;
    bool        recovering;
} OCParser;

static inline int precedence(OCParser* parser) {
    return orbit_tokenBinaryPrecedence(parser->lexer.currentToken.kind);
}

static inline bool rightAssoc(OCParser* parser) {
    return orbit_tokenBinaryRightAssoc(parser->lexer.currentToken.kind);
}

static inline OrbitToken current(OCParser* parser) {
    return parser->lexer.currentToken;
}

//void compilerError(OCParser* parser, const char* format, int count, ...);

bool have(OCParser* parser, OrbitTokenKind kind);

bool match(OCParser* parser, OrbitTokenKind kind);

// MARK: - utility functions, mainly used to avoid typing long [have()] lists
bool haveBinaryOp(OCParser* parser);

bool haveUnaryOp(OCParser* parser);

bool haveConditional(OCParser* parser);

bool haveTerm(OCParser* parser);

bool havePrimitiveType(OCParser* parser);

bool haveType(OCParser* parser);

bool implicitTerminator(OCParser* parser);

bool expectTerminator(OCParser* parser);

bool expect(OCParser* parser, OrbitTokenKind kind);

#endif /* orbit_parser_recdescent_h */
