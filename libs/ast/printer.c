//===--------------------------------------------------------------------------------------------===
// orbit/ast/orbit_astPrinter.c
// This source is part of Orbit - AST
//
// Created on 2017-09-26 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdio.h>
#include <stdbool.h>
#include <orbit/csupport/console.h>
#include <orbit/ast/ast.h>

static void orbit_astPrintNode(FILE* out, OrbitAST* ast, int depth, bool last);
static void orbit_astPrintList(FILE* out, const char* name, OrbitAST* list, int depth, bool last);

static void orbit_astPrintReturn(FILE* out, int depth, bool last) {
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

static void orbit_astPrintList(FILE* out, const char* name, OrbitAST* list, int depth, bool last) {
    if(list == NULL) { return; }
    orbit_astPrintReturn(out, depth, last);
    
    console_setColor(out, CLI_CYAN);
    console_setColor(out, CLI_BOLD);
    fprintf(out, "%s", name);
    console_setColor(out, CLI_RESET);
    
    OrbitAST* item = list;
    while(item != NULL) {
        orbit_astPrintNode(out, item, depth+1, item->next == NULL);
        item = item->next;
    }
}

static void orbit_astPrintType(FILE* out, OrbitAST* ast) {
    if(ast == NULL) { return; }
    if((ast->kind & ASTTypeExprMask) == 0) { return; }
    
    if((ast->typeExpr.flags & ORBIT_TYPE_OPTIONAL)) {
        fputs("maybe ", out);
    }
    
    switch(ast->kind) {
    case ORBIT_AST_TYPEEXPR_VOID:     fputs("Void", out);     break;
    case ORBIT_AST_TYPEEXPR_BOOL:     fputs("Bool", out);     break;
    case ORBIT_AST_TYPEEXPR_NUMBER:   fputs("Number", out);   break;
    case ORBIT_AST_TYPEEXPR_STRING:   fputs("String", out);   break;
    case ORBIT_AST_TYPEEXPR_ANY:      fputs("Any", out);      break;
    case ORBIT_AST_TYPEEXPR_USER:
        console_printPooledString(out, ast->typeExpr.userType.symbol);
        break;
        
    case ORBIT_AST_TYPEEXPR_FUNC:
        fputs("(", out);
        orbit_astPrintType(out, ast->typeExpr.funcType.params);
        fputs(") -> ", out);
        orbit_astPrintType(out, ast->typeExpr.funcType.returnType);
        break;
        
    case ORBIT_AST_TYPEEXPR_ARRAY:
        fputs("Array<", out);
        orbit_astPrintType(out, ast->typeExpr.arrayType.elementType);
        fputs(">", out);
        break;
        
    case ORBIT_AST_TYPEEXPR_MAP:
        fputs("Map<", out);
        orbit_astPrintType(out, ast->typeExpr.mapType.keyType);
        fputs(":", out);
        orbit_astPrintType(out, ast->typeExpr.mapType.elementType);
        fputs(">", out);
        break;
        
    default:
        break;
    }
    
    if(ast->next) {
        fputs(", ", out);
        orbit_astPrintType(out, ast->next);
    }
}

static void orbit_astPrintNode(FILE* out, OrbitAST* ast, int depth, bool last) {
    if(ast == NULL) { return; }
    orbit_astPrintReturn(out, depth, last);
    
    console_setColor(out, CLI_BOLD);
    
    switch(ast->kind) {
    case ORBIT_AST_CONDITIONAL:
        console_setColor(out, CLI_MAGENTA);
        fputs("IfStmt", out);
        orbit_astPrintNode(out, ast->conditionalStmt.condition, depth+1, false);
        orbit_astPrintNode(out, ast->conditionalStmt.ifBody, depth+1, ast->conditionalStmt.elseBody == NULL);
        //orbit_astPrintList(out, "Block", ast->conditionalStmt.ifBody, depth+1, ast->conditionalStmt.elseBody == NULL);
        orbit_astPrintNode(out, ast->conditionalStmt.elseBody, depth+1, true);
        //orbit_astPrintList(out, "Block", ast->conditionalStmt.elseBody, depth+1, true);
        break;
    
    case ORBIT_AST_FOR_IN:
        console_setColor(out, CLI_MAGENTA);
        fputs("ForInStmt ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->forInLoop.variable);
        orbit_astPrintNode(out, ast->forInLoop.collection, depth+1, false);
        orbit_astPrintNode(out, ast->forInLoop.body, depth+1, true);
        //orbit_astPrintList(out, "Block", ast->forInLoop.body, depth+1, true);
        break;
    
    case ORBIT_AST_WHILE:
        console_setColor(out, CLI_MAGENTA);
        fputs("WhileStmt", out);
        orbit_astPrintNode(out, ast->whileLoop.condition, depth+1, false);
        orbit_astPrintNode(out, ast->whileLoop.body, depth+1, true);
        //orbit_astPrintList(out, "Block", ast->whileLoop.body, depth+1, true);
        break;
        
    case ORBIT_AST_BLOCK:
        console_setColor(out, CLI_CYAN);
        console_setColor(out, CLI_BOLD);
        fputs("Block", out);
        console_setColor(out, CLI_RESET);
        
        OrbitAST* item = ast->block.body;
        while(item != NULL) {
            orbit_astPrintNode(out, item, depth+1, item->next == NULL);
            item = item->next;
        }
        //orbit_astPrintList(out, "Block", ast->block.body, depth+1, true);
        break;
    
    case ORBIT_AST_BREAK:
        console_setColor(out, CLI_MAGENTA);
        fputs("BreakStmt", out);
        break;
        
    case ORBIT_AST_CONTINUE:
        console_setColor(out, CLI_MAGENTA);
        fputs("ContinueStmt", out);
        break;
        
    case ORBIT_AST_RETURN:
        console_setColor(out, CLI_MAGENTA);
        fputs("ReturnStmt", out);
        orbit_astPrintNode(out, ast->returnStmt.returnValue, depth+1, true);
        break;
    
    // DECLARATIONS
    case ORBIT_AST_DECL_MODULE:
        console_setColor(out, CLI_GREEN);
        fputs("ModuleDecl '", out);
        console_printPooledString(out, ast->moduleDecl.symbol);
        fputs("'", out);
        orbit_astPrintList(out, "Block", ast->moduleDecl.body, depth+1, true);
        break;
    
    case ORBIT_AST_DECL_FUNC:
        console_setColor(out, CLI_GREEN);
        fputs("FuncDecl ", out);
        console_setColor(out, CLI_RESET);
        console_printPooledString(out, ast->funcDecl.name);
        fputs(" (", out);
        console_setColor(out, CLI_GREEN);
        console_printPooledString(out, ast->funcDecl.mangledName);
        console_setColor(out, CLI_RESET);
        fputs("): ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        orbit_astPrintType(out, ast->type);
        orbit_astPrintList(out, "ParamDeclList", ast->funcDecl.params, depth+1, false);
        orbit_astPrintNode(out, ast->funcDecl.body, depth+1, true);
        break;
    
    case ORBIT_AST_DECL_VAR:
        console_setColor(out, CLI_GREEN);
        fputs("VarDecl ", out);
        console_setColor(out, CLI_RESET);
        console_printPooledString(out, ast->varDecl.name);
        fputs(": ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        orbit_astPrintType(out, ast->type);
        break;
    
    case ORBIT_AST_DECL_STRUCT:
        console_setColor(out, CLI_GREEN);
        fputs("CompoundTypeDecl ", out);
        console_setColor(out, CLI_RESET);
        console_printPooledString(out, ast->structDecl.name);
        orbit_astPrintNode(out, ast->structDecl.constructor, depth+1, false);
        orbit_astPrintNode(out, ast->structDecl.destructor, depth+1, false);
        orbit_astPrintList(out, "CompoundMemberList", ast->structDecl.fields, depth+1, true);
        break;
        
    // EXPRESSIONS
    case ORBIT_AST_EXPR_UNARY:
        console_setColor(out, CLI_MAGENTA);
        fputs("UnaryOperatorExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->unaryExpr.operator);
        fputs(": ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        orbit_astPrintType(out, ast->type);
        orbit_astPrintNode(out, ast->unaryExpr.rhs, depth+1, true);
        break;
    
    case ORBIT_AST_EXPR_BINARY:
        console_setColor(out, CLI_MAGENTA);
        fputs("BinaryOperatorExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->binaryExpr.operator);
        fputs(": ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        orbit_astPrintType(out, ast->type);
        orbit_astPrintNode(out, ast->binaryExpr.lhs, depth+1, false);
        orbit_astPrintNode(out, ast->binaryExpr.rhs, depth+1, true);
        break;
    
    case ORBIT_AST_EXPR_CALL:
        console_setColor(out, CLI_MAGENTA);
        fputs("CallExpr", out);
        orbit_astPrintNode(out, ast->callExpr.symbol, depth+1, ast->callExpr.params == NULL);
        orbit_astPrintList(out, "CallParamList", ast->callExpr.params, depth+1, true);
        break;
        
    case ORBIT_AST_EXPR_SUBSCRIPT:
        console_setColor(out, CLI_MAGENTA);
        fputs("SubscriptExpr", out);
        orbit_astPrintNode(out, ast->subscriptExpr.symbol, depth+1, false);
        orbit_astPrintNode(out, ast->subscriptExpr.subscript, depth+1, true);
        break;
    
    case ORBIT_AST_EXPR_CONSTANT_INTEGER:
        console_setColor(out, CLI_MAGENTA);
        fputs("IntegerLiteralExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(": ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        orbit_astPrintType(out, ast->type);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT_FLOAT:
        console_setColor(out, CLI_MAGENTA);
        fputs("FloatLiteralExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(": ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        orbit_astPrintType(out, ast->type);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT_STRING:
        console_setColor(out, CLI_MAGENTA);
        fputs("StringLiteralExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(" (", out); console_printPooledString(out, ast->constantExpr.symbol.parsedStringLiteral); fputs(")", out);
        fputs(": ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        orbit_astPrintType(out, ast->type);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT:
        console_setColor(out, CLI_MAGENTA);
        fputs("ConstantExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(": ", out);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        orbit_astPrintType(out, ast->type);
        break;
        
    case ORBIT_AST_EXPR_LAMBDA:
        console_setColor(out, CLI_MAGENTA);
        fputs("lambda: ", out);
        console_setColor(out, CLI_RESET);
        fputs(": ", out);
        orbit_astPrintList(out, "ParamList", ast->lambdaExpr.params, depth+1, false);
        orbit_astPrintNode(out, ast->lambdaExpr.body, depth+1, true);
        break;
    
    case ORBIT_AST_EXPR_NAME:
        console_setColor(out, CLI_MAGENTA);
        fputs("NameRefExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printPooledString(out, ast->nameExpr.name);
        break;
    
    case ORBIT_AST_EXPR_INIT:
        console_setColor(out, CLI_MAGENTA);
        fputs("init:", out);
        console_setColor(out, CLI_RESET);
        console_setColor(out, CLI_YELLOW); console_setColor(out, CLI_BOLD);
        orbit_astPrintType(out, ast->initExpr.type); // TODO: replace with sema-generated type
        console_setColor(out, CLI_RESET);
        orbit_astPrintList(out, "ConstructorList", ast->callExpr.params, depth+1, true);
        break;
    
    case ORBIT_AST_TYPEEXPR_VOID:
    case ORBIT_AST_TYPEEXPR_BOOL:
    case ORBIT_AST_TYPEEXPR_NUMBER:
    case ORBIT_AST_TYPEEXPR_STRING:
    case ORBIT_AST_TYPEEXPR_USER:
    case ORBIT_AST_TYPEEXPR_ANY:
    case ORBIT_AST_TYPEEXPR_FUNC:
    case ORBIT_AST_TYPEEXPR_ARRAY:
    case ORBIT_AST_TYPEEXPR_MAP:
        orbit_astPrintType(out, ast);
        break;
    }
    console_setColor(out, CLI_RESET);
}

void orbit_astPrint(FILE* out, OrbitAST* ast) {
    if(ast == NULL) { return; }
    if(ast->next) {
        orbit_astPrintList(out, "List", ast, 0, true);
    } else {
        orbit_astPrintNode(out, ast, 0, true);
    }
    fputs("\n\n", out);
}
