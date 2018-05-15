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

static OrbitAST* recBlock(OCParser*);
static OrbitAST* recTypeDecl(OCParser*);
static OrbitAST* recVarDecl(OCParser*);
static OrbitAST* recFuncDecl(OCParser*);
static OrbitAST* recParameters(OCParser*);

static OrbitAST* recStatement(OCParser*);
static OrbitAST* recConditional(OCParser*);
static OrbitAST* recIfStatement(OCParser*);
static OrbitAST* recWhileLoop(OCParser*);
static OrbitAST* recForLoop(OCParser*);
static OrbitAST* recFlowStatement(OCParser*);
static OrbitAST* recReturnStatement(OCParser*);

static OrbitAST* recExpression(OCParser*, int);
static OrbitAST* recTerm(OCParser*);
static OrbitAST* recName(OCParser*);
static OrbitAST* recSubscript(OCParser*);
static OrbitAST* recFieldAccess(OCParser*);
static OrbitAST* recFuncCall(OCParser*);
static OrbitAST* recExprList(OCParser*);

static OrbitAST* recType(OCParser*);
static OrbitAST* recFuncType(OCParser*);
static OrbitAST* recTypename(OCParser*);
static OrbitAST* recPrimitive(OCParser*);
static OrbitAST* recArrayType(OCParser*);
static OrbitAST* recMapType(OCParser*);

#endif /* orbit_parser_recognizers_h */
