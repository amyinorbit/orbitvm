//
//  orbit/parser/recursive_descent.h
//  Orbit - Parser
//
//  Created by Amy Parent on 2017-10-23.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_parser_recdescent_h
#define orbit_parser_recdescent_h

#include <orbit/parser/compiler_utils.h>
#include <orbit/parser/tokens.h>

typedef struct {
    OCLexer     lexer;
    bool        recovering;
} OCParser;

static inline int precedence(OCParser* parser) {
    return orbit_binaryPrecedence(parser->lexer.currentToken.type);
}

static inline bool rightAssoc(OCParser* parser) {
    return orbit_binaryRightAssoc(parser->lexer.currentToken.type);
}

static inline OCToken current(OCParser* parser) {
    return parser->lexer.currentToken;
}

void compilerError(OCParser* parser, const char* fmt, ...);

void syntaxError(OCParser* parser, OCTokenType type);

bool have(OCParser* parser, OCTokenType type);

bool match(OCParser* parser, OCTokenType type);

// MARK: - utility functions, mainly used to avoid typing long [have()] lists
bool haveBinaryOp(OCParser* parser);

bool haveUnaryOp(OCParser* parser);

bool haveConditional(OCParser* parser);

bool haveTerm(OCParser* parser);

bool havePrimitiveType(OCParser* parser);

bool haveType(OCParser* parser);

bool implicitTerminator(OCParser* parser);

bool expectTerminator(OCParser* parser);

bool expect(OCParser* parser, OCTokenType type);

#endif /* orbit_parser_recdescent_h */
