//===--------------------------------------------------------------------------------------------===
// orbit/ast/ast.h
// This source is part of Orbit - AST
//
// Created on 2017-09-16 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
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
    AST*    head;
    AST**   next;
};

void ast_listStart(ASTListBuilder* builder);
void ast_listAdd(ASTListBuilder* builder, AST* item);
AST* ast_listClose(ASTListBuilder* builder);

/// AST Node builders

AST* ast_makeConditional(AST* condition, AST* ifBody, AST* elseBody);
AST* ast_makeForInLoop(const OCToken* var, AST* collection, AST* body);
AST* ast_makeWhileLoop(AST* condition, AST* body);
AST* ast_makeBreak();
AST* ast_makeContinue();
AST* ast_makeReturn(AST* returned);

AST* ast_makeModuleDecl(const char* symbol, AST* body);
AST* ast_makeStructDecl(const OCToken* symbol, AST* constructor, AST* destructor, AST* fields);
AST* ast_makeVarDecl(const OCToken* symbol, AST* typeAnnotation);
AST* ast_makeFuncDecl(const OCToken* symbol, AST* returnType, AST* params, AST* body);

AST* ast_makeBinaryExpr(const OCToken* operator, AST* lhs, AST* rhs);
AST* ast_makeUnaryExpr(const OCToken* operator, AST* rhs);
AST* ast_makeCallExpr(AST* symbol, AST* params);
AST* ast_makeSubscriptExpr(AST* symbol, AST* subscript);
AST* ast_makeNameExpr(const OCToken* symbol);
AST* ast_makeConstantExpr(const OCToken* symbol, ASTKind kind);

// AST* ast_makePrimitiveType(ASTKind kind);
AST* ast_makeUserType(const OCToken* symbol);
AST* ast_makeUserTypePooled(OCStringID symbol);
AST* ast_makePrimitiveType(ASTKind kind);
AST* ast_makeFuncType(AST* returnType, AST* params);
AST* ast_makeArrayType(AST* elementType);
AST* ast_makeMapType(AST* keyType, AST* elementType);

#endif /* orbit_ast_builders_h_ */
