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

void orbit_astListStart(ASTListBuilder* builder);
void orbit_astListAdd(ASTListBuilder* builder, OrbitAST* item);
OrbitAST* orbit_astListClose(ASTListBuilder* builder);

/// AST Node builders

OrbitAST* orbit_astMakeConditional(OrbitAST* condition, OrbitAST* ifBody, OrbitAST* elseBody);
OrbitAST* orbit_astMakeForInLoop(const OrbitToken* var, OrbitAST* collection, OrbitAST* body);
OrbitAST* orbit_astMakeWhileLoop(OrbitAST* condition, OrbitAST* body);
OrbitAST* orbit_astMakeBlock(OrbitAST* body);
OrbitAST* orbit_astMakeBreak();
OrbitAST* orbit_astMakeContinue();
OrbitAST* orbit_astMakeReturn(OrbitAST* returned);

OrbitAST* orbit_astMakeModuleDecl(const char* symbol, OrbitAST* body);
OrbitAST* orbit_astMakeStructDecl(const OrbitToken* symbol, OrbitAST* constructor, OrbitAST* destructor, OrbitAST* fields);
OrbitAST* orbit_astMakeVarDecl(const OrbitToken* symbol, OrbitAST* typeAnnotation);
OrbitAST* orbit_astMakeFuncDecl(const OrbitToken* symbol, OrbitAST* returnType, OrbitAST* params, OrbitAST* body);

OrbitAST* orbit_astMakeBinaryExpr(const OrbitToken* operator, OrbitAST* lhs, OrbitAST* rhs);
OrbitAST* orbit_astMakeUnaryExpr(const OrbitToken* operator, OrbitAST* rhs);
OrbitAST* orbit_astMakeCallExpr(OrbitAST* symbol, OrbitAST* params);
OrbitAST* orbit_astMakeSubscriptExpr(OrbitAST* symbol, OrbitAST* subscript);
OrbitAST* orbit_astMakeNameExpr(const OrbitToken* symbol);
OrbitAST* orbit_astMakeConstantExpr(const OrbitToken* symbol, ASTKind kind);
OrbitAST* orbit_astMakeInitExpr(OrbitAST* type, OrbitAST* params);

// OrbitAST* orbit_astMakePrimitiveType(ASTKind kind);
OrbitAST* orbit_astMakeUserType(const OrbitToken* symbol);
OrbitAST* orbit_astMakeUserTypePooled(OCStringID symbol);
OrbitAST* orbit_astMakePrimitiveType(ASTKind kind);
OrbitAST* orbit_astMakeFuncType(OrbitAST* returnType, OrbitAST* params);
OrbitAST* orbit_astMakeArrayType(OrbitAST* elementType);
OrbitAST* orbit_astMakeMapType(OrbitAST* keyType, OrbitAST* elementType);

#endif /* orbit_ast_builders_h_ */
