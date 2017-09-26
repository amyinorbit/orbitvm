//
//  orbit/parser/recognizers.h
//  Orbit - Parser
//
//  Created by Amy Parent on 2017-06-20.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_parser_recognizers_h
#define orbit_parser_recognizers_h
#include <orbit/ast/ast.h>

typedef struct {
    OCLexer     lexer;
    bool        recovering;
} OCParser;

static AST* recBlock(OCParser*);
static AST* recTypeDecl(OCParser*);
static AST* recVarDecl(OCParser*);
static AST* recFuncDecl(OCParser*);
static AST* recParameters(OCParser*);

static AST* recStatement(OCParser*);
static AST* recConditional(OCParser*);
static AST* recIfStatement(OCParser*);
static AST* recWhileLoop(OCParser*);
static AST* recForLoop(OCParser*);
static AST* recFlowStatement(OCParser*);
static AST* recReturnStatement(OCParser*);

static AST* recExpression(OCParser*, int);
static AST* recTerm(OCParser*);
static AST* recName(OCParser*);
static AST* recSubscript(OCParser*);
static AST* recFieldAccess(OCParser*);
static AST* recFuncCall(OCParser*);
static AST* recExprList(OCParser*);

static AST* recType(OCParser*);
static AST* recFuncType(OCParser*);
static AST* recTypename(OCParser*);
static AST* recPrimitive(OCParser*);
static AST* recArrayType(OCParser*);
static AST* recMapType(OCParser*);

#endif /* orbit_parser_recognizers_h */
