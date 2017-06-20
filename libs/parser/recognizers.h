//
//  orbit/parser/recognizers.h
//  Orbit - Parser
//
//  Created by Amy Parent on 2017-06-20.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_parser_recognizers_h
#define orbit_parser_recognizers_h

typedef struct {
    OCLexer     lexer;
    bool        recovering;
} OCParser;

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

#endif /* orbit_parser_recognizers_h */
