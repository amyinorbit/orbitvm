//
//  orbit/type/type.h
//  Orbit - AST
//
//  Created by Amy Parent on 2017-10-06.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_type_h
#define orbit_type_h

#include <stdio.h>
#include <stdbool.h>
#include <orbit/source/tokens.h>
#include <orbit/utils/platforms.h>

typedef enum _TypeKind TypeKind;
typedef struct _Type Type;

enum _TypeKind {
    TYPE_NIL    = 1 << 0,
    TYPE_VOID   = 1 << 1,
    TYPE_BOOL   = 1 << 2,
    TYPE_NUMBER = 1 << 3,
    TYPE_STRING = 1 << 4,
    TYPE_FUNC   = 1 << 5,
    TYPE_ARRAY  = 1 << 6,
    TYPE_MAP    = 1 << 7,
    TYPE_USER   = 1 << 8,
    TYPE_ANY    = 0xffffffff ,
};

/// The internal representation (in the compiler anyway) of an Orbit type.
struct _Type {
    TypeKind        kind;
    bool            isConst;
    Type*           next;
    
    union {
        struct {
            Type*   valueType;
        } array;
        
        struct {
            Type*   keyType;
            Type*   valueType;
        } map;
        
        struct {
            Type*   returnType;
            Type*   params;
        } function;
        
        struct {
            OCToken symbol;
            Type*   members;
        } user;
    };
};

/// Checks whether [typeA] and [typeB] are strictly the same type (no aliases).
bool type_strictEquals(Type* typeA, Type* typeB);

/// Prints a string representation of [type] to [out].
void type_print(FILE* out, Type* type);

Type* type_make(TypeKind kind, bool isConst);
Type* type_copy(Type* type);
void type_destroy(Type* type);

#endif /* orbit_type_h */
