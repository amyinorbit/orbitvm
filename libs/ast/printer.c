//
//  orbit/ast/ast_printer.c
//  Orbit - AST
//
//  Created by Amy Parent on 2017-09-26.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdio.h>
#include <stdbool.h>
#include <orbit/console/console.h>
#include <orbit/ast/ast.h>
#include <orbit/type/type.h>

static void ast_printNode(FILE* out, AST* ast, int depth, bool last);
static void ast_printList(FILE* out, const char* name, AST* list, int depth, bool last);

static void ast_printReturn(FILE* out, int depth, bool last) {
    static bool indents[256] = {false};
    fputs("\n", out);
    if(depth <= 0) { return; }
    
    console_setColor(out, CLI_RESET);
    console_setColor(out, CLI_BLUE);
    indents[depth-1] = !last;
    for(int i = 0; i < depth-1; ++i) {
        fputc(((i >= 256 || indents[i]) ? '|' : ' '), out);
        fputc(' ', out);
    }
    fputs((last ? "`-" : "|-"), out);
    console_setColor(out, CLI_RESET);
}

static void ast_printList(FILE* out, const char* name, AST* list, int depth, bool last) {
    if(list == NULL) { return; }
    ast_printReturn(out, depth, last);
    
    console_setColor(out, CLI_CYAN);
    console_setColor(out, CLI_BOLD);
    fprintf(out, "%s", name);
    console_setColor(out, CLI_RESET);
    
    AST* item = list;
    while(item != NULL) {
        ast_printNode(out, item, depth+1, item->next == NULL);
        item = item->next;
    }
}

static void ast_printNode(FILE* out, AST* ast, int depth, bool last) {
    if(ast == NULL) { return; }
    ast_printReturn(out, depth, last);
    
    console_setColor(out, CLI_BOLD);
    
    switch(ast->kind) {
    case AST_CONDITIONAL:
        console_setColor(out, CLI_MAGENTA);
        fputs("IfStmt", out);
        ast_printNode(out, ast->conditionalStmt.condition, depth+1, false);
        ast_printList(out, "Block", ast->conditionalStmt.ifBody, depth+1, ast->conditionalStmt.elseBody == NULL);
        ast_printList(out, "Block", ast->conditionalStmt.elseBody, depth+1, true);
        break;
    
    case AST_FOR_IN:
        console_setColor(out, CLI_MAGENTA);
        fputs("ForInStmt", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->forInLoop.variable);
        ast_printNode(out, ast->forInLoop.collection, depth+1, false);
        ast_printList(out, "Block", ast->forInLoop.body, depth+1, true);
        break;
    
    case AST_WHILE:
        console_setColor(out, CLI_MAGENTA);
        fputs("WhileStmt", out);
        ast_printNode(out, ast->whileLoop.condition, depth+1, false);
        ast_printList(out, "Block", ast->whileLoop.body, depth+1, true);
        break;
    
    case AST_BREAK:
        console_setColor(out, CLI_MAGENTA);
        fputs("BreakStmt", out);
        break;
        
    case AST_CONTINUE:
        console_setColor(out, CLI_MAGENTA);
        fputs("ContinueStmt", out);
        break;
        
    case AST_RETURN:
        console_setColor(out, CLI_MAGENTA);
        fputs("ReturnStmt", out);
        ast_printNode(out, ast->returnStmt.returnValue, depth+1, true);
        break;
    
    // DECLARATIONS
    case AST_DECL_MODULE:
        console_setColor(out, CLI_GREEN);
        fprintf(out, "ModuleDecl '%s'", ast->moduleDecl.symbol);
        ast_printList(out, "DeclarationList", ast->moduleDecl.body, depth+1, true);
        break;
    
    case AST_DECL_FUNC:
        console_setColor(out, CLI_GREEN);
        fputs("FuncDecl ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->funcDecl.symbol);
        fputs(": ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        type_print(out, ast->type);
        ast_printList(out, "ParamDeclList", ast->funcDecl.params, depth+1, false);
        ast_printList(out, "Block", ast->funcDecl.body, depth+1, true);
        break;
    
    case AST_DECL_VAR:
        console_setColor(out, CLI_GREEN);
        fputs("VarDecl ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->varDecl.symbol);
        fputs(": ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        type_print(out, ast->type);
        break;
    
    case AST_DECL_STRUCT:
        console_setColor(out, CLI_GREEN);
        fputs("CompoundTypeDecl ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->structDecl.symbol);
        ast_printNode(out, ast->structDecl.constructor, depth+1, false);
        ast_printNode(out, ast->structDecl.destructor, depth+1, false);
        ast_printList(out, "CompoundMemberList", ast->structDecl.fields, depth+1, true);
        break;
        
    // EXPRESSIONS
    case AST_EXPR_UNARY:
        console_setColor(out, CLI_MAGENTA);
        fputs("UnaryOperatorExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->unaryExpr.operator);
        fputs(": ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        type_print(out, ast->type);
        ast_printNode(out, ast->unaryExpr.rhs, depth+1, true);
        break;
    
    case AST_EXPR_BINARY:
        console_setColor(out, CLI_MAGENTA);
        fputs("BinaryOperatorExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->binaryExpr.operator);
        fputs(": ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        type_print(out, ast->type);
        ast_printNode(out, ast->binaryExpr.lhs, depth+1, false);
        ast_printNode(out, ast->binaryExpr.rhs, depth+1, true);
        break;
    
    case AST_EXPR_CALL:
        console_setColor(out, CLI_MAGENTA);
        fputs("CallExpr", out);
        ast_printNode(out, ast->callExpr.symbol, depth+1, ast->callExpr.params == NULL);
        ast_printList(out, "CallParamList", ast->callExpr.params, depth+1, true);
        break;
        
    case AST_EXPR_SUBSCRIPT:
        console_setColor(out, CLI_MAGENTA);
        fputs("SubscriptExpr", out);
        ast_printNode(out, ast->subscriptExpr.symbol, depth+1, false);
        ast_printNode(out, ast->subscriptExpr.subscript, depth+1, true);
        break;
    
    case AST_EXPR_CONSTANT_INTEGER:
        console_setColor(out, CLI_MAGENTA);
        fputs("IntegerLiteralExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(": ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        type_print(out, ast->type);
        break;
        
    case AST_EXPR_CONSTANT_FLOAT:
        console_setColor(out, CLI_MAGENTA);
        fputs("FloatLiteralExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(": ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        type_print(out, ast->type);
        break;
        
    case AST_EXPR_CONSTANT_STRING:
        console_setColor(out, CLI_MAGENTA);
        fputs("StringLiteralExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(": ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        type_print(out, ast->type);
        break;
        
    case AST_EXPR_CONSTANT:
        console_setColor(out, CLI_MAGENTA);
        fputs("ConstantExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(": ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        type_print(out, ast->type);
        break;
    
    case AST_EXPR_NAME:
        console_setColor(out, CLI_MAGENTA);
        fputs("NameRefExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->nameExpr.symbol);
        break;
    
    case AST_TYPEEXPR_SIMPLE:
        console_setColor(out, CLI_MAGENTA);
        fputs("TypeExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->simpleType.symbol);
        break;
        
    case AST_TYPEEXPR_FUNC:
        console_setColor(out, CLI_MAGENTA);
        fputs("FuncTypeExpr", out);
        ast_printNode(out, ast->funcType.returnType, depth+1, ast->funcType.params == NULL);
        ast_printList(out, "ParamList", ast->funcType.params, depth+1, true);
        break;
        
    case AST_TYPEEXPR_ARRAY:
        console_setColor(out, CLI_MAGENTA);
        fputs("ArrayTypeExpr", out);
        ast_printNode(out, ast->arrayType.elementType, depth+1, false);
        break;
        
    case AST_TYPEEXPR_MAP:
        console_setColor(out, CLI_MAGENTA);
        fputs("MapTypeExpr", out);
        ast_printNode(out, ast->mapType.keyType, depth+1, false);
        ast_printNode(out, ast->mapType.elementType, depth+1, true);
        break;
    }
    console_setColor(out, CLI_RESET);
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
