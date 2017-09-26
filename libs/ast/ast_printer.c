//
//  orbit/ast/ast_printer.c
//  Orbit - AST
//
//  Created by Amy Parent on 2017-09-26.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdio.h>
#include <orbit/ast/ast.h>

static void ast_printReturn(FILE* out, int depth) {
    fputs("\n", out);
    for(int i = 0; i < depth; ++i) {
        fputs("   ", out);
    }
    fputs("|", out);
}

static void ast_printToken(FILE* out, OCToken* token) {
    fputs("`", out);
    const char* current = token->start;
    for(uint64_t i = 0; i < token->length; ++i) {
        fputc(*(current++), out);
    }
    fputs("`", out);
    //fprintf(out, "`%*.s`", (int)token->length, token->start);
}

static void ast_printNode(FILE* out, const char* prefix, AST* ast, int depth) {
    if(ast == NULL) { return; }
    ast_printReturn(out, depth);
    fprintf(out, "_%s", prefix);
    
    switch(ast->type) {
    case AST_LIST:
        ast_printNode(out, "", ast->list.head, depth+1);
        break;
        
    case AST_CONDITIONAL:
        fprintf(out, "conditional");
        ast_printNode(out, "condition:", ast->conditionalStmt.condition, depth+1);
        ast_printNode(out, "if-body:", ast->conditionalStmt.ifBody, depth+1);
        ast_printNode(out, "else-body:", ast->conditionalStmt.elseBody, depth+1);
        break;
    
    case AST_FOR_IN:
        fprintf(out, "for-in ");
        ast_printToken(out, ast->forInLoop.variable);
        ast_printNode(out, "collection:", ast->forInLoop.collection, depth+1);
        ast_printNode(out, "body:", ast->forInLoop.body, depth+1);
        break;
    
    case AST_WHILE:
        fprintf(out, "while");
        ast_printNode(out, "condition:", ast->whileLoop.condition, depth+1);
        ast_printNode(out, "body:", ast->whileLoop.body, depth+1);
        break;
    
    // DECLARATIONS
    case AST_DECL_MODULE:
        fprintf(out, "module-decl `%s`", ast->moduleDecl.symbol);
        ast_printNode(out, "body:", ast->moduleDecl.body, depth+1);
        break;
    
    case AST_DECL_FUNC:
        fprintf(out, "func-decl ");
        ast_printToken(out, ast->funcDecl.symbol);
        ast_printNode(out, "params:", ast->funcDecl.params, depth+1);
        ast_printNode(out, "returns:", ast->funcDecl.returnType, depth+1);
        ast_printNode(out, "body:", ast->funcDecl.body, depth+1);
        break;
    
    case AST_DECL_VAR:
        fprintf(out, "var-decl ");
        ast_printToken(out, ast->varDecl.symbol);
        ast_printNode(out, "type:", ast->varDecl.typeAnnotation, depth+1);
        break;
    
    case AST_DECL_PARAM:
        fprintf(out, "param-decl ");
        ast_printToken(out, ast->paramDecl.symbol);
        ast_printNode(out, "type:", ast->paramDecl.typeAnnotation, depth+1);
        break;
    
    case AST_DECL_STRUCT:
        fprintf(out, "usertype-decl");
        ast_printToken(out, ast->structDecl.symbol);
        ast_printNode(out, "constructor:", ast->structDecl.constructor, depth+1);
        ast_printNode(out, "destructor:", ast->structDecl.destructor, depth+1);
        ast_printNode(out, "members:", ast->structDecl.fields, depth+1);
        break;
        
    // EXPRESSIONS
    case AST_EXPR_UNARY:
        ast_printToken(out, ast->unaryExpr.operator);
        ast_printNode(out, "", ast->unaryExpr.rhs, depth+1);
        break;
    
    case AST_EXPR_BINARY:
        ast_printToken(out, ast->binaryExpr.operator);
        ast_printNode(out, "", ast->binaryExpr.lhs, depth+1);
        ast_printNode(out, "", ast->binaryExpr.rhs, depth+1);
        break;
    
    case AST_EXPR_CALL:
        fprintf(out, "call ");
        ast_printNode(out, "symbol:", ast->callExpr.symbol, depth+1);
        ast_printNode(out, "params:", ast->callExpr.params, depth+1);
        break;
        
    case AST_EXPR_SUBSCRIPT:
        fprintf(out, "subscript ");
        ast_printNode(out, "symbol:", ast->subscriptExpr.symbol, depth+1);
        ast_printNode(out, "subscript:", ast->subscriptExpr.subscript, depth+1);
        break;
    
    case AST_EXPR_CONSTANT:
        fprintf(out, "constant ");
        ast_printToken(out, ast->constantExpr.symbol);
        break;
    
    case AST_EXPR_NAME:
        fprintf(out, "symbol ");
        ast_printToken(out, ast->nameExpr.symbol);
        break;
    
    case AST_EXPR_TYPE:
        fprintf(out, "type-expr ");
        ast_printToken(out, ast->typeExpr.symbol);
        break;
    }
    
    ast_printNode(out, "", ast->next, depth);
}

void ast_print(FILE* out, AST* ast) {
    ast_printNode(out, "", ast, 0);
    fputs("\n\n", out);
}
