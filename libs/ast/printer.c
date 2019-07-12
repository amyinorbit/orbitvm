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
#include <term/colors.h>

static void orbitASTPrintNode(FILE* out, const OrbitAST* ast, int depth, bool last);
static void orbitASTPrintList(FILE* out, const char* name, const OrbitAST* list, int depth, bool last);

static void orbitASTPrintReturn(FILE* out, int depth, bool last) {
    static bool indents[256] = {false};
    fputs("\n", out);
    if(depth <= 0) { return; }
    
    termReset(out);
    termColorFG(out, kTermBlue);
    indents[depth-1] = !last;
    for(int i = 0; i < depth-1; ++i) {
        fputc(((i >= 256 || indents[i]) ? '|' : ' '), out);
        fputc(' ', out);
    }
    fputs((last ? "`-" : "|-"), out);
    termReset(out);
}

static void orbitASTPrintList(FILE* out, const char* name, const OrbitAST* list, int depth, bool last) {
    if(list == NULL) { return; }
    orbitASTPrintReturn(out, depth, last);
    
    termColorFG(out, kTermCyan);
    termBold(out, true);
    fprintf(out, "%s", name);
    termReset(out);
    
    while(list != NULL) {
        orbitASTPrintNode(out, list, depth+1, list->next == NULL);
        list = list->next;
    }
}

static void orbitASTPrintType(FILE* out, const OrbitAST* ast) {
    if(ast == NULL) { return; }
    if((ast->kind & ASTTypeExprMask) == 0) { return; }
    termColorFG(out, kTermYellow);
    
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

static void orbitASTPrintNode(FILE* out, const OrbitAST* ast, int depth, bool last) {
    if(ast == NULL) { return; }
    orbitASTPrintReturn(out, depth, last);
    termBold(out, true);
    
    switch(ast->kind) {
    case ORBIT_AST_ASSIGN:
        termColorFG(out, kTermMagenta);
        fputs("AssignStmt ", out);
        termReset(out);
        console_printToken(out, ast->assignStmt.operator);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        orbitASTPrintNode(out, ast->assignStmt.lhs, depth+1, false);
        orbitASTPrintNode(out, ast->assignStmt.rhs, depth+1, true);
        break;
            
    case ORBIT_AST_CONDITIONAL:
        termColorFG(out, kTermMagenta);
        fputs("IfStmt", out);
        orbitASTPrintNode(out, ast->conditionalStmt.condition, depth+1, false);
        orbitASTPrintNode(out, ast->conditionalStmt.ifBody, depth+1, ast->conditionalStmt.elseBody == NULL);
        //orbitASTPrintList(out, "Block", ast->conditionalStmt.ifBody, depth+1, ast->conditionalStmt.elseBody == NULL);
        orbitASTPrintNode(out, ast->conditionalStmt.elseBody, depth+1, true);
        //orbitASTPrintList(out, "Block", ast->conditionalStmt.elseBody, depth+1, true);
        break;
    
    case ORBIT_AST_FOR_IN:
        termColorFG(out, kTermMagenta);
        fputs("ForInStmt ", out);
        termReset(out);
        console_printToken(out, ast->forInLoop.variable);
        orbitASTPrintNode(out, ast->forInLoop.collection, depth+1, false);
        orbitASTPrintNode(out, ast->forInLoop.body, depth+1, true);
        //orbitASTPrintList(out, "Block", ast->forInLoop.body, depth+1, true);
        break;
    
    case ORBIT_AST_WHILE:
        termColorFG(out, kTermMagenta);
        fputs("WhileStmt", out);
        orbitASTPrintNode(out, ast->whileLoop.condition, depth+1, false);
        orbitASTPrintNode(out, ast->whileLoop.body, depth+1, true);
        //orbitASTPrintList(out, "Block", ast->whileLoop.body, depth+1, true);
        break;
        
    case ORBIT_AST_BLOCK:
        termColorFG(out, kTermCyan);
        termBold(out, true);
        fputs("Block", out);
        termReset(out);
        
        OrbitAST* item = ast->block.body;
        while(item != NULL) {
            orbitASTPrintNode(out, item, depth+1, item->next == NULL);
            item = item->next;
        }
        //orbitASTPrintList(out, "Block", ast->block.body, depth+1, true);
        break;
    
    case ORBIT_AST_BREAK:
        termColorFG(out, kTermMagenta);
        fputs("BreakStmt", out);
        break;
        
    case ORBIT_AST_CONTINUE:
        termColorFG(out, kTermMagenta);
        fputs("ContinueStmt", out);
        break;
        
    case ORBIT_AST_RETURN:
        termColorFG(out, kTermMagenta);
        fputs("ReturnStmt", out);
        orbitASTPrintNode(out, ast->returnStmt.returnValue, depth+1, true);
        break;
        
    case ORBIT_AST_PRINT:
        termColorFG(out, kTermMagenta);
        fputs("PrintStmt", out);
        orbitASTPrintNode(out, ast->printStmt.expr, depth+1, true);
        break;
    
    // DECLARATIONS
    case ORBIT_AST_DECL_MODULE:
        termColorFG(out, kTermGreen);
        fputs("ModuleDecl '", out);
        console_printPooledString(out, ast->moduleDecl.symbol);
        fputs("'", out);
        orbitASTPrintList(out, "Block", ast->moduleDecl.body, depth+1, true);
        break;
    
    case ORBIT_AST_DECL_FUNC:
        termColorFG(out, kTermGreen);
        fputs("FuncDecl ", out);
        termReset(out);
        console_printPooledString(out, ast->funcDecl.name);
        fputs(" (", out);
        termColorFG(out, kTermGreen);
        console_printPooledString(out, ast->funcDecl.mangledName);
        termReset(out);
        fputs("): ", out);
        orbitASTPrintType(out, ast->type);
        orbitASTPrintList(out, "ParamDeclList", ast->funcDecl.params, depth+1, false);
        orbitASTPrintNode(out, ast->funcDecl.body, depth+1, true);
        break;
    
    case ORBIT_AST_DECL_VAR:
        termColorFG(out, kTermGreen);
        fputs("VarDecl ", out);
        termReset(out);
        console_printPooledString(out, ast->varDecl.name);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        break;
    
    case ORBIT_AST_DECL_STRUCT:
        termColorFG(out, kTermGreen);
        fputs("CompoundTypeDecl ", out);
        termReset(out);
        console_printPooledString(out, ast->structDecl.name);
        orbitASTPrintNode(out, ast->structDecl.constructor, depth+1, false);
        orbitASTPrintNode(out, ast->structDecl.destructor, depth+1, false);
        orbitASTPrintList(out, "CompoundMemberList", ast->structDecl.fields, depth+1, true);
        break;
        
    // EXPRESSIONS
    case ORBIT_AST_EXPR_UNARY:
        termColorFG(out, kTermMagenta);
        fputs("UnaryOperatorExpr ", out);
        termReset(out);
        console_printToken(out, ast->unaryExpr.operator);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        orbitASTPrintNode(out, ast->unaryExpr.rhs, depth+1, true);
        break;
    
    case ORBIT_AST_EXPR_BINARY:
        termColorFG(out, kTermMagenta);
        fputs("BinaryOperatorExpr ", out);
        termReset(out);
        console_printToken(out, ast->binaryExpr.operator);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        orbitASTPrintNode(out, ast->binaryExpr.lhs, depth+1, false);
        orbitASTPrintNode(out, ast->binaryExpr.rhs, depth+1, true);
        break;
    
    case ORBIT_AST_EXPR_CALL:
        termColorFG(out, kTermMagenta);
        fputs("CallExpr", out);
        termReset(out);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        orbitASTPrintNode(out, ast->callExpr.symbol, depth+1, ast->callExpr.params == NULL);
        orbitASTPrintList(out, "CallParamList", ast->callExpr.params, depth+1, true);
        break;
        
    case ORBIT_AST_EXPR_SUBSCRIPT:
        termColorFG(out, kTermMagenta);
        fputs("SubscriptExpr", out);
        orbitASTPrintNode(out, ast->subscriptExpr.symbol, depth+1, false);
        orbitASTPrintNode(out, ast->subscriptExpr.subscript, depth+1, true);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT_BOOL:
        termColorFG(out, kTermMagenta);
        fputs("BoolLiteralExpr ", out);
        termReset(out);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        break;
    
    case ORBIT_AST_EXPR_CONSTANT_INTEGER:
        termColorFG(out, kTermMagenta);
        fputs("IntegerLiteralExpr ", out);
        termReset(out);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT_FLOAT:
        termColorFG(out, kTermMagenta);
        fputs("FloatLiteralExpr ", out);
        termReset(out);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT_STRING:
        termColorFG(out, kTermMagenta);
        fputs("StringLiteralExpr ", out);
        termReset(out);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(" (", out); console_printPooledString(out, ast->constantExpr.symbol.parsedStringLiteral); fputs(")", out);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        break;
        
    case ORBIT_AST_EXPR_CONSTANT:
        termColorFG(out, kTermMagenta);
        fputs("ConstantExpr ", out);
        termReset(out);
        console_printToken(out, ast->constantExpr.symbol);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        break;
        
    case ORBIT_AST_EXPR_LAMBDA:
        termColorFG(out, kTermMagenta);
        fputs("lambda: ", out);
        termReset(out);
        fputs(": ", out);
        orbitASTPrintList(out, "ParamList", ast->lambdaExpr.params, depth+1, false);
        orbitASTPrintNode(out, ast->lambdaExpr.body, depth+1, true);
        break;
    
    case ORBIT_AST_EXPR_NAME:
        termColorFG(out, kTermMagenta);
        fputs("NameRefExpr ", out);
        termReset(out);
        console_printPooledString(out, ast->nameExpr.name);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        break;
    
    case ORBIT_AST_EXPR_INIT:
        termColorFG(out, kTermMagenta);
        fputs("init:", out);
        termReset(out);
        orbitASTPrintType(out, ast->initExpr.type); // TODO: replace with sema-generated type
        termReset(out);
        orbitASTPrintList(out, "ConstructorList", ast->callExpr.params, depth+1, true);
        break;
        
    case ORBIT_AST_EXPR_I2F:
        termColorFG(out, kTermMagenta);
        fputs("IntToFloat", out);
        termReset(out);
        fputs(": ", out);
        orbitASTPrintType(out, ast->type);
        orbitASTPrintNode(out, ast->conversionExpr.expr, depth+1, true);
        break;
    
    case ORBIT_AST_EXPR_F2I:
        termColorFG(out, kTermMagenta);
        fputs("FloatToInt", out);
        termReset(out);
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
    termReset(out);
}

void orbitASTPrint(FILE* out, const OrbitAST* ast) {
    if(ast == NULL) { return; }
    if(ast->next) {
        orbitASTPrintList(out, "List", ast, 0, true);
    } else {
        orbitASTPrintNode(out, ast, 0, true);
    }
    fputs("\n\n", out);
}
