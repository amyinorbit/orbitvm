//===--------------------------------------------------------------------------------------------===
// orbit/parser/recognizers.h
// This source is part of Orbit - Parser
//
// Created on 2017-06-20 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_parser_recognizers_h
#define orbit_parser_recognizers_h
#include <orbit/ast/ast.h>
#include "recursive_descent.h"

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
