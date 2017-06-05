//
//  orbit_ast.h
//  OrbitVM
//
//  Created by Amy Parent on 2017-05-24.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_ast_h
#define orbit_adt_h

typedef struct _OCAstNode OCAstNode;
typedef enum {
    AST_STRING_CONSTANT,
    AST_NUMBER_CONSTANT,
    AST_FUNCTION_CALL,
    AST_FUNCTION_DECL,
    AST_EXPRESSION_UNARY,
    AST_EXPRESSION_BINARY,
    AST_EXPRESSION_CALL,
    AST_EXPRESSION_SUBSCRIPT
} OCAstType;

typedef struct _OCAstNode {
    OCAstNode*      parent;
    OCAstNode*      next;
    OCAstType       type;
    OCToken         sourceToken;
    union {
        double      numberConstant;
        const char* stringConstant;
        OCAstNode*  childList;
    };
};

#endif /* orbit_adt_h */
