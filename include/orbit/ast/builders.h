//===--------------------------------------------------------------------------------------------===
// orbit/ast/ast.h
// This source is part of Orbit - AST
//
// Created on 2017-09-16 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_ast_builders_h
#define orbit_ast_builders_h

#include <stdio.h>
#include <orbit/ast/ast.h>

/// Helper functions and types used to simplify list/block building

typedef struct _ASTListBuilder ASTListBuilder;

struct _ASTListBuilder {
    OrbitAST*    head;
    OrbitAST**   next;
};

void orbitASTListStart(ASTListBuilder* builder);
void orbitASTListAdd(ASTListBuilder* builder, OrbitAST* item);
OrbitAST* orbitASTListClose(ASTListBuilder* builder);

/// AST Node builders

OrbitAST* orbitASTMakeAssign(const OrbitToken* operator, OrbitAST* lhs, OrbitAST* rhs);
OrbitAST* orbitASTMakeConditional(OrbitAST* condition, OrbitAST* ifBody, OrbitAST* elseBody);
OrbitAST* orbitASTMakeForInLoop(const OrbitToken* var, OrbitAST* collection, OrbitAST* body);
OrbitAST* orbitASTMakeWhileLoop(OrbitAST* condition, OrbitAST* body);
OrbitAST* orbitASTMakeBlock(OrbitAST* body);
OrbitAST* orbitASTMakeBreak();
OrbitAST* orbitASTMakeContinue();
OrbitAST* orbitASTMakeReturn(OrbitAST* returned);
OrbitAST* orbitASTMakePrint(OrbitAST* expr);

OrbitAST* orbitASTMakeModuleDecl(const char* symbol, OrbitAST* body);
OrbitAST* orbitASTMakeStructDecl(const OrbitToken* symbol, OrbitAST* constructor, OrbitAST* destructor, OrbitAST* fields);
OrbitAST* orbitASTMakeVarDecl(const OrbitToken* symbol, OrbitAST* typeAnnotation);
OrbitAST* orbitASTMakeFuncDecl(const OrbitToken* symbol, OrbitAST* returnType, OrbitAST* params, OrbitAST* body);

OrbitAST* orbitASTMakeBinaryExpr(const OrbitToken* operator, OrbitAST* lhs, OrbitAST* rhs);
OrbitAST* orbitASTMakeUnaryExpr(const OrbitToken* operator, OrbitAST* rhs);
OrbitAST* orbitASTMakeCallExpr(OrbitAST* symbol, OrbitAST* params);
OrbitAST* orbitASTMakeSubscriptExpr(OrbitAST* symbol, OrbitAST* subscript);
OrbitAST* orbitASTMakeNameExpr(const OrbitToken* symbol);
OrbitAST* orbitASTMakeConstantExpr(const OrbitToken* symbol, ASTKind kind);
OrbitAST* orbitASTMakeInitExpr(OrbitAST* type, OrbitAST* params);
OrbitAST* orbitASTMakeLambdaExpr(OrbitAST* params, OrbitAST* body);
OrbitAST* orbitASTMakeCastExpr(OrbitAST* expr, ASTKind kind);
OrbitAST* orbitASTMakeI2F(OrbitAST* expr);
OrbitAST* orbitASTMakeF2I(OrbitAST* expr);

// OrbitAST* ,(ASTKind kind);
OrbitAST* orbitASTMakeUserType(const OrbitToken* symbol);
OrbitAST* orbitASTMakeUserTypePooled(OCStringID symbol);
OrbitAST* orbitASTMakePrimitiveType(ASTKind kind);
OrbitAST* orbitASTMakeFuncType(OrbitAST* returnType, OrbitAST* params);
OrbitAST* orbitASTMakeArrayType(OrbitAST* elementType);
OrbitAST* orbitASTMakeMapType(OrbitAST* keyType, OrbitAST* elementType);
OrbitAST* orbitASTMakeOptional(OrbitAST* type, bool isOptional);
OrbitAST* orbitASTMakeConst(OrbitAST* type, bool isConst);
OrbitAST* orbitASTSetLValue(OrbitAST* type, bool isLValue);

#endif /* orbit_ast_builders_h_ */
