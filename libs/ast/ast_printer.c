//
//  orbit/ast/ast_printer.c
//  Orbit - AST
//
//  Created by Amy Parent on 2017-09-26.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdio.h>
#include <stdbool.h>
#include <orbit/ast/ast.h>

static void ast_printNode(FILE* out, AST* ast, int depth, bool last);
static void ast_printList(FILE* out, const char* name, AST* list, int depth, bool last);

static void ast_printReturn(FILE* out, int depth, bool last) {
    
    static bool indents[256] = {false};
    
    indents[depth] = !last;
    
    fputs("\n", out);
    for(int i = 0; i < depth; ++i) {
        fputc(((i >= 256 || indents[i]) ? '|' : ' '), out);
        fputc(' ', out);
    }
    fputs((last ? "`-" : "|-"), out);
}

static void ast_printToken(FILE* out, OCToken token) {
    fputs("'", out);
    const char* current = token.start;
    for(uint64_t i = 0; i < token.length; ++i) {
        fputc(*(current++), out);
    }
    fputs("'", out);
}

static void ast_printList(FILE* out, const char* name, AST* list, int depth, bool last) {
    if(list == NULL) { return; }
    ast_printReturn(out, depth, last);
    fprintf(out, "%s", name);
    
    AST* item = list;
    while(item != NULL) {
        ast_printNode(out, item, depth+1, item->next == NULL);
        item = item->next;
    }
}

static void ast_printNode(FILE* out, AST* ast, int depth, bool last) {
    if(ast == NULL) { return; }
    ast_printReturn(out, depth, last);
    
    switch(ast->type) {
    case AST_LIST:
        fputs("NodeList", out);
        ast_printNode(out, ast->list.head, depth+1, false);
        break;
        
    case AST_CONDITIONAL:
        fputs("IfStmt", out);
        ast_printNode(out, ast->conditionalStmt.condition, depth+1, false);
        ast_printList(out, "Block", ast->conditionalStmt.ifBody, depth+1, false);
        ast_printList(out, "Block", ast->conditionalStmt.elseBody, depth+1, true);
        break;
    
    case AST_FOR_IN:
        fputs("ForInStmt", out);
        ast_printToken(out, ast->forInLoop.variable);
        ast_printNode(out, ast->forInLoop.collection, depth+1, false);
        ast_printList(out, "Block", ast->forInLoop.body, depth+2, true);
        break;
    
    case AST_WHILE:
        fputs("WhileStmt", out);
        ast_printNode(out, ast->whileLoop.condition, depth+1, false);
        ast_printList(out, "Block", ast->whileLoop.body, depth+1, true);
        break;
    
    case AST_BREAK:
        fputs("BreakStmt", out);
        break;
        
    case AST_CONTINUE:
        fputs("ContinueStmt", out);
        break;
        
    case AST_RETURN:
        fputs("ReturnStmt", out);
        ast_printNode(out, ast->returnStmt.returnValue, depth+1, true);
        break;
    
    // DECLARATIONS
    case AST_DECL_MODULE:
        fprintf(out, "ModuleDecl `%s`", ast->moduleDecl.symbol);
        ast_printList(out, "Block", ast->moduleDecl.body, depth+1, true);
        break;
    
    case AST_DECL_FUNC:
        fputs("FuncDecl ", out);
        ast_printToken(out, ast->funcDecl.symbol);
        ast_printNode(out, ast->funcDecl.params, depth+1, false);
        ast_printNode(out, ast->funcDecl.returnType, depth+1, false);
        ast_printList(out, "Block", ast->funcDecl.body, depth+1, true);
        break;
    
    case AST_DECL_VAR:
        fputs("VarDecl ", out);
        ast_printToken(out, ast->varDecl.symbol);
        ast_printNode(out, ast->varDecl.typeAnnotation, depth+1, true);
        break;
    
    case AST_DECL_STRUCT:
        fputs("CompoundTypeDecl ", out);
        ast_printToken(out, ast->structDecl.symbol);
        ast_printNode(out, ast->structDecl.constructor, depth+1, false);
        ast_printNode(out, ast->structDecl.destructor, depth+1, false);
        ast_printList(out, "CompoundMemberList", ast->structDecl.fields, depth+1, true);
        break;
        
    // EXPRESSIONS
    case AST_EXPR_UNARY:
        fputs("UnaryOperatorExpr ", out);
        ast_printToken(out, ast->unaryExpr.operator);
        ast_printNode(out, ast->unaryExpr.rhs, depth+1, true);
        break;
    
    case AST_EXPR_BINARY:
        fputs("BinaryOperatorExpr ", out);
        ast_printToken(out, ast->binaryExpr.operator);
        ast_printNode(out, ast->binaryExpr.lhs, depth+1, false);
        ast_printNode(out, ast->binaryExpr.rhs, depth+1, true);
        break;
    
    case AST_EXPR_CALL:
        fputs("CallExpr", out);
        ast_printNode(out, ast->callExpr.symbol, depth+1, false);
        ast_printList(out, "CallParamList", ast->callExpr.params, depth+1, true);
        break;
        
    case AST_EXPR_SUBSCRIPT:
        fputs("SubscriptExpr", out);
        ast_printNode(out, ast->subscriptExpr.symbol, depth+1, false);
        ast_printNode(out, ast->subscriptExpr.subscript, depth+1, true);
        break;
    
    case AST_EXPR_CONSTANT:
        fputs("ConstantExpr ", out);
        ast_printToken(out, ast->constantExpr.symbol);
        break;
    
    case AST_EXPR_NAME:
        fputs("NameRefExpr ", out);
        ast_printToken(out, ast->nameExpr.symbol);
        break;
    
    case AST_TYPEEXPR_SIMPLE:
        fputs("TypeExpr ", out);
        ast_printToken(out, ast->simpleType.symbol);
        break;
        
    case AST_TYPEEXPR_FUNC:
        fputs("FuncTypeExpr", out);
        ast_printNode(out, ast->funcType.returnType, depth+1, false);
        ast_printList(out, "ParamList", ast->funcType.params, depth+1, true);
        break;
        
    case AST_TYPEEXPR_ARRAY:
        fputs("ArrayTypeExpr", out);
        ast_printNode(out, ast->arrayType.elementType, depth+1, false);
        break;
        
    case AST_TYPEEXPR_MAP:
        fputs("MapTypeExpr", out);
        ast_printNode(out, ast->mapType.keyType, depth+1, false);
        ast_printNode(out, ast->mapType.elementType, depth+1, true);
        break;
    }
    
    //ast_printNode(out, ast->next, depth, false);
}

void ast_print(FILE* out, AST* ast) {
    if(ast == NULL) { return; }
    if(ast->next) {
        ast_printList(out, "List", ast, 0, true);
    } else {
        ast_printNode(out, ast, 0, true);
    }
    fputs("\n\n", out);
}
