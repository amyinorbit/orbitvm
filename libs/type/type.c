//
//  orbit/type/type.c
//  Orbit - AST
//
//  Created by Amy Parent on 2017-10-06.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <stdlib.h>
#include <string.h>
#include <orbit/type/type.h>

bool type_strictEquals(Type* typeA, Type* typeB) {
    if(typeA == NULL) { return false; }
    if(typeB == NULL) { return false; }
    if(typeA == typeB) { return true; }
    return false;
}

void type_print(FILE* out, Type* type) {
    if(type == NULL) { return; }
    switch(type->kind) {
    case TYPE_NIL:
        fputs("Nil", out);
        break;
        
    case TYPE_VOID:
        fputs("Void", out);
        break;
        
    case TYPE_NUMBER:
        fputs("Number", out);
        break;
        
    case TYPE_STRING:
        fputs("String", out);
        break;
        
    case TYPE_FUNC:
        fputs("(", out);
        type_print(out, type->function.params);
        fputs(") -> ", out);
        type_print(out, type->function.returnType);
        break;
        
    case TYPE_ARRAY:
        fputs("Array[", out);
        type_print(out, type->array.valueType);
        fputs("]", out);
        break;
        
    case TYPE_MAP:
        fputs("Map[", out);
        type_print(out, type->map.keyType);
        fputs(":", out);
        type_print(out, type->map.valueType);
        fputs("]", out);
        break;
        
    case TYPE_ANY:
        fputs("Any", out);
        break;
        
    case TYPE_USER:
        break;
    }
    
    if(type->next) {
        fputs(", ", out);
        type_print(out, type->next);
    }
}

Type* type_make(TypeKind kind, bool isConst) {
    // TODO: Maybe let's not malloc everytime. Some sort of pool system maybe?
    Type* t = malloc(sizeof (Type));
    memset(t, 0, sizeof (Type));
    
    t->kind = kind;
    t->isConst = isConst;
    t->next = NULL;
    return t;
}

Type* type_copy(Type* type) {
    if(type == NULL) { return NULL; }
    
    Type* copy = type_make(type->kind, type->isConst);
    
    switch(type->kind) {
    case TYPE_NIL:
    case TYPE_VOID:
    case TYPE_NUMBER:
    case TYPE_STRING:
    case TYPE_ANY:
        break;
        
    case TYPE_FUNC:
        copy->function.params = type_copy(type->function.params);
        copy->function.returnType = type_copy(type->function.returnType);
        break;
        
    case TYPE_ARRAY:
        copy->array.valueType = type_copy(type->array.valueType);
        break;
        
    case TYPE_MAP:
        copy->map.keyType = type_copy(type->map.keyType);
        copy->map.valueType = type_copy(type->map.valueType);
        break;
        
    case TYPE_USER:
        copy->user.members = type_copy(type->user.members);
        break;
    }
    
    copy->next = type_copy(type->next);
    return copy;
}

void type_destroy(Type* type) {
    if(type == NULL) { return; }
    switch(type->kind) {
    case TYPE_NIL:
    case TYPE_VOID:
    case TYPE_NUMBER:
    case TYPE_STRING:
    case TYPE_ANY:
        break;
        
    case TYPE_FUNC:
        type_destroy(type->function.params);
        type_destroy(type->function.returnType);
        break;
        
    case TYPE_ARRAY:
        type_destroy(type->array.valueType);
        break;
        
    case TYPE_MAP:
        type_destroy(type->map.keyType);
        type_destroy(type->map.valueType);
        break;
        
    case TYPE_USER:
        type_destroy(type->user.members);
        break;
    }
    type_destroy(type->next);
    free(type);
}

Type* type_makeArray(Type* elementType) {
    Type* type = type_make(TYPE_ARRAY, false);
    type->array.valueType = elementType;
    return type;
}

Type* type_makeMap(Type* keyType, Type* elementType) {
    Type* type = type_make(TYPE_MAP, false);
    type->map.keyType = keyType;
    type->map.valueType = elementType;
    return type;
}

Type* type_makeFunction(Type* returnType, Type* paramTypes) {
    Type* type = type_make(TYPE_FUNC, false);
    type->function.returnType = returnType;
    type->function.params = paramTypes;
    return type;
}

Type* type_makeUserType(OCToken* symbol) {
    Type* type = type_make(TYPE_USER, false);
    return type;
}
