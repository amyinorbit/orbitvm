//===--------------------------------------------------------------------------------------------===
// orbit/ast/orbitASTPrinter.c
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

static void orbitASTPrintNode(FILE* out, OrbitAST* ast, int depth, bool last);
static void orbitASTPrintList(FILE* out, const char* name, OrbitAST* list, int depth, bool last);

static void orbitASTPrintReturn(FILE* out, int depth, bool last) {
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

static void orbitASTPrintList(FILE* out, const char* name, OrbitAST* list, int depth, bool last) {
    if(list == NULL) { return; }
    orbitASTPrintReturn(out, depth, last);
    
    console_setColor(out, CLI_CYAN);
    console_setColor(out, CLI_BOLD);
    fprintf(out, "%s", name);
    console_setColor(out, CLI_RESET);
    
    OrbitAST* item = list;
    while(item != NULL) {
        orbitASTPrintNode(out, item, depth+1, item->next == NULL);
        item = item->next;
    }
}

static void orbitASTPrintType(FILE* out, OrbitAST* ast) {
    if(ast == NULL) { return; }
    if((ast->kind & ASTTypeExprMask) == 0) { return; }
    console_setColor(out, CLI_YELLOW);
    
    if((ast->typeExpr.flags & ORBIT_TYPE_OPTIONAL)) {
        fputs("maybe ", out);
    }
    
    switch(ast->kind) {
    case ORBIT_AST_TYPEEXPR_VOID:       fputs("()", out);       break;
    case ORBIT_AST_TYPEEXPR_BOOL:       fputs("Bool", out);     break;
    case ORBIT_AST_TYPEEXPR_INT:        fputs("Int", out);      break;
    case ORBIT_AST_TYPEEXPR_FLOAT:      fputs("Float", out);    break;
    case ORBIT_AST_TYPEEXPR_STRING:     fputs("String", out);   break;
    case ORBIT_AST_TYPEEXPR_ANY:        fputs("Any", out);      break;
    case ORBIT_AST_TYPEEXPR_USER:
        console_printPooledString(out, ast->typeExpr.userType.symbol);
        break;
        
    case ORBIT_AST_TYPEEXPR_FUNC:
        fputs("", out);
        orbitASTPrintType(out, ast->typeExpr.funcType.params);
        fputs(" -> ", out);
        orbitASTPrintType(out, ast->typeExpr.funcType.returnType);
        break;
        
    case ORBIT_AST_TYPEEXPR_ARRAY:
        fputs("Array<", out);
        orbitASTPrintType(out, ast->typeExpr.arrayType.elementType);
        fputs(">", out);
        break;
        
    case ORBIT_AST_TYPEEXPR_MAP:
        fputs("Map<", out);
        orbitASTPrintType(out, ast->typeExpr.mapType.keyType);
        fputs(":", out);
        orbitASTPrintType(out, ast->typeExpr.mapType.elementType);
        fputs(">", out);
        break;
        
    default:
        break;
    }
    
    if(ast->next) {
        fputs(", ", out);
        orbitASTPrintType(out, ast->next);
    }
}

static void orbitASTPrintNode(FILE* out, OrbitAST* ast, int depth, bool last) {
    if(ast == NULL) { return; }
    orbitASTPrintReturn(out, depth, last);
    console_setColor(out, CLI_BOLD);
    
    switch(ast->kind) {
    case ORBIT_AST_ASSIGN:
        console_setColor(out, CLI_MAGENTA);
        fputs("AssignStmt ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->assignStmt.operator);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        orbitASTPrintNode(out, ast->assignStmt.lhs, depth+1, false);
        orbitASTPrintNode(out, ast->assignStmt.rhs, depth+1, true);
        break;
            
    case ORBIT_AST_CONDITIONAL:
        console_setColor(out, CLI_MAGENTA);
        fputs("IfStmt", out);
        orbitASTPrintNode(out, ast->conditionalStmt.condition, depth+1, false);
        orbitASTPrintNode(out, ast->conditionalStmt.ifBody, depth+1, ast->conditionalStmt.elseBody == NULL);
        //orbitASTPrintList(out, "Block", ast->conditionalStmt.ifBody, depth+1, ast->conditionalStmt.elseBody == NULL);
        orbitASTPrintNode(out, ast->conditionalStmt.elseBody, depth+1, true);
        //orbitASTPrintList(out, "Block", ast->conditionalStmt.elseBody, depth+1, true);
        break;
    
    case ORBIT_AST_FOR_IN:
        console_setColor(out, CLI_MAGENTA);
        fputs("ForInStmt ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->forInLoop.variable);
        orbitASTPrintNode(out, ast->forInLoop.collection, depth+1, false);
        orbitASTPrintNode(out, ast->forInLoop.body, depth+1, true);
        //orbitASTPrintList(out, "Block", ast->forInLoop.body, depth+1, true);
        break;
    
    case ORBIT_AST_WHILE:
        console_setColor(out, CLI_MAGENTA);
        fputs("WhileStmt", out);
        orbitASTPrintNode(out, ast->whileLoop.condition, depth+1, false);
        orbitASTPrintNode(out, ast->whileLoop.body, depth+1, true);
        //orbitASTPrintList(out, "Block", ast->whileLoop.body, depth+1, true);
        break;
        
    case ORBIT_AST_BLOCK:
        console_setColor(out, CLI_CYAN);
        console_setColor(out, CLI_BOLD);
        fputs("Block", out);
        console_setColor(out, CLI_RESET);
        
        OrbitAST* item = ast->block.body;
        while(item != NULL) {
            orbitASTPrintNode(out, item, depth+1, item->next == NULL);
            item = item->next;
        }
        //orbitASTPrintList(out, "Block", ast->block.body, depth+1, true);
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
        orbitASTPrintNode(out, ast->returnStmt.returnValue, depth+1, true);
        break;
        
    case ORBIT_AST_PRINT:
        console_setColor(out, CLI_MAGENTA);
        fputs("PrintStmt", out);
        orbitASTPrintNode(out, ast->printStmt.expr, depth+1, true);
        break;
    
    // DECLARATIONS
    case ORBIT_AST_DECL_MODULE:
        console_setColor(out, CLI_GREEN);
        fputs("ModuleDecl '", out);
        console_printPooledString(out, ast->moduleDecl.symbol);
        fputs("'", out);
        orbitASTPrintList(out, "Block", ast->moduleDecl.body, depth+1, true);
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
        orbitASTPrintType(out, ast->type);
        orbitASTPrintList(out, "ParamDeclList", ast->funcDecl.params, depth+1, false);
        orbitASTPrintNode(out, ast->funcDecl.body, depth+1, true);
        break;
    
    case ORBIT_AST_DECL_VAR:
        console_setColor(out, CLI_GREEN);
        fputs("VarDecl ", out);
        console_setColor(out, CLI_RESET);
        console_printPooledString(out, ast->varDecl.name);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        break;
    
    case ORBIT_AST_DECL_STRUCT:
        console_setColor(out, CLI_GREEN);
        fputs("CompoundTypeDecl ", out);
        console_setColor(out, CLI_RESET);
        console_printPooledString(out, ast->structDecl.name);
        orbitASTPrintNode(out, ast->structDecl.constructor, depth+1, false);
        orbitASTPrintNode(out, ast->structDecl.destructor, depth+1, false);
        orbitASTPrintList(out, "CompoundMemberList", ast->structDecl.fields, depth+1, true);
        break;
        
    // EXPRESSIONS
    case ORBIT_AST_EXPR_UNARY:
        console_setColor(out, CLI_MAGENTA);
        fputs("UnaryOperatorExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->unaryExpr.operator);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        orbitASTPrintNode(out, ast->unaryExpr.rhs, depth+1, true);
        break;
    
    case ORBIT_AST_EXPR_BINARY:
        console_setColor(out, CLI_MAGENTA);
        fputs("BinaryOperatorExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->binaryExpr.operator);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        orbitASTPrintNode(out, ast->binaryExpr.lhs, depth+1, false);
        orbitASTPrintNode(out, ast->binaryExpr.rhs, depth+1, true);
        break;
    
    case ORBIT_AST_EXPR_CALL:
        console_setColor(out, CLI_MAGENTA);
        fputs("CallExpr", out);
        console_setColor(out, CLI_RESET);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        orbitASTPrintNode(out, ast->callExpr.symbol, depth+1, ast->callExpr.params == NULL);
        orbitASTPrintList(out, "CallParamList", ast->callExpr.params, depth+1, true);
        break;
        
    case ORBIT_AST_EXPR_SUBSCRIPT:
        console_setColor(out, CLI_MAGENTA);
        fputs("SubscriptExpr", out);
        orbitASTPrintNode(out, ast->subscriptExpr.symbol, depth+1, false);
        orbitASTPrintNode(out, ast->subscriptExpr.subscript, depth+1, true);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT_BOOL:
        console_setColor(out, CLI_MAGENTA);
        fputs("BoolLiteralExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        break;
    
    case ORBIT_AST_EXPR_CONSTANT_INTEGER:
        console_setColor(out, CLI_MAGENTA);
        fputs("IntegerLiteralExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT_FLOAT:
        console_setColor(out, CLI_MAGENTA);
        fputs("FloatLiteralExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT_STRING:
        console_setColor(out, CLI_MAGENTA);
        fputs("StringLiteralExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(" (", out); console_printPooledString(out, ast->constantExpr.symbol.parsedStringLiteral); fputs(")", out);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT:
        console_setColor(out, CLI_MAGENTA);
        fputs("ConstantExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        break;
        
    case ORBIT_AST_EXPR_LAMBDA:
        console_setColor(out, CLI_MAGENTA);
        fputs("lambda: ", out);
        console_setColor(out, CLI_RESET);
        fputs(": ", out);
        orbitASTPrintList(out, "ParamList", ast->lambdaExpr.params, depth+1, false);
        orbitASTPrintNode(out, ast->lambdaExpr.body, depth+1, true);
        break;
    
    case ORBIT_AST_EXPR_NAME:
        console_setColor(out, CLI_MAGENTA);
        fputs("NameRefExpr ", out);
        console_setColor(out, CLI_RESET);
        console_printPooledString(out, ast->nameExpr.name);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        break;
    
    case ORBIT_AST_EXPR_INIT:
        console_setColor(out, CLI_MAGENTA);
        fputs("init:", out);
        console_setColor(out, CLI_RESET);
        orbitASTPrintType(out, ast->initExpr.type); // TODO: replace with sema-generated type
        console_setColor(out, CLI_RESET);
        orbitASTPrintList(out, "ConstructorList", ast->callExpr.params, depth+1, true);
        break;
        
    case ORBIT_AST_EXPR_I2F:
        console_setColor(out, CLI_MAGENTA);
        fputs("IntToFloat", out);
        console_setColor(out, CLI_RESET);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        orbitASTPrintNode(out, ast->conversionExpr.expr, depth+1, true);
        break;
    
    case ORBIT_AST_EXPR_F2I:
        console_setColor(out, CLI_MAGENTA);
        fputs("FloatToInt", out);
        console_setColor(out, CLI_RESET);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        orbitASTPrintNode(out, ast->conversionExpr.expr, depth+1, true);
        break;
    
    case ORBIT_AST_TYPEEXPR_VOID:
    case ORBIT_AST_TYPEEXPR_BOOL:
    case ORBIT_AST_TYPEEXPR_INT:
    case ORBIT_AST_TYPEEXPR_FLOAT:
    case ORBIT_AST_TYPEEXPR_STRING:
    case ORBIT_AST_TYPEEXPR_USER:
    case ORBIT_AST_TYPEEXPR_ANY:
    case ORBIT_AST_TYPEEXPR_FUNC:
    case ORBIT_AST_TYPEEXPR_ARRAY:
    case ORBIT_AST_TYPEEXPR_MAP:
        orbitASTPrintType(out, ast);
        break;
    }
    console_setColor(out, CLI_RESET);
}

void orbitASTPrint(FILE* out, OrbitAST* ast) {
    if(ast == NULL) { return; }
    if(ast->next) {
        orbitASTPrintList(out, "List", ast, 0, true);
    } else {
        orbitASTPrintNode(out, ast, 0, true);
    }
    fputs("\n\n", out);
}
