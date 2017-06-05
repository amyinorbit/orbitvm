//
//  orbit/parser/type.c
//  Orbit - Parser
//
//  Created by Amy Parent on 2017-05-26.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <orbit/parser/type.h>

#define TYPE_DEFAULT_CAPACITY (64)

static OCType*  _types = NULL;
static uint32_t _typeCount = 0;
static uint32_t _typeCapacity = 0;

OCType* orbit_Void = &MAKE_TYPE(TYPE_VOID);
OCType* orbit_Bool = &MAKE_TYPE(TYPE_BOOL);
OCType* orbit_Number = &MAKE_TYPE(TYPE_NUMBER);
OCType* orbit_String = &MAKE_TYPE(TYPE_STRING);

static const char* typeNames[] = {
    [TYPE_VOID] = "Void",
    [TYPE_NUMBER] = "Number",
    [TYPE_STRING] = "String",
    [TYPE_ARRAY] = "Array",
    [TYPE_MAP] = "Map",
    [TYPE_FUNCTION] = "",
    [TYPE_USER] = "type",
};

static void _ensureTypeTable() {
    if(_typeCount < _typeCapacity) { return; }
    if(_typeCapacity == 0) {
        _typeCapacity = TYPE_DEFAULT_CAPACITY;
    } else {
        _typeCapacity *= 2;
    }
    _types = realloc(_types, _typeCapacity * sizeof(OCType));
}

static void _installPrimitives() {
    orbit_Void = orbit_typeInstall(&MAKE_TYPE(TYPE_VOID));
    orbit_Bool = orbit_typeInstall(&MAKE_TYPE(TYPE_BOOL));
    orbit_Number = orbit_typeInstall(&MAKE_TYPE(TYPE_NUMBER));
    orbit_String = orbit_typeInstall(&MAKE_TYPE(TYPE_STRING));
}

static void _uninstallPrimitives() {
    orbit_Void = NULL;
    orbit_Bool = NULL;
    orbit_Number = NULL;
    orbit_String = NULL;
}

void orbit_typeCheckerInit() {
    _typeCount = 0;
    _typeCapacity = 0;
    _ensureTypeTable();
    _installPrimitives();
}

void orbit_typeCheckerDeinit() {
    _typeCount = 0;
    _typeCapacity = 0;
    free(_types);
    _types = NULL;
    _uninstallPrimitives();
}

void orbit_typePrint(FILE* out, const OCType* type) {
    if(type->name) {
        fprintf(out, "%s:", type->name);
    }
    fprintf(out, "%s", typeNames[type->kind]);
    if(type->children) {
        fprintf(out, type->kind == TYPE_FUNCTION ? "(" : "[");
        orbit_typePrint(out, type->children);
        fprintf(out, type->kind == TYPE_FUNCTION ? ")" : "]");
    }
    if(type->sibling) {
        fprintf(out, ", ");
        orbit_typePrint(out, type->sibling);
    }
    if(type->returnType) {
        fprintf(out, " -> ");
        orbit_typePrint(out, type->returnType);
    }
}

bool orbit_typeEqual(const OCType* typeA, const OCType* typeB) {
    if(typeA == typeB) { return true; }
    if(typeA == NULL || typeB == NULL) { return false; }
    
    return typeA->kind == typeB->kind
        && typeA->isConst == typeB->isConst
        && typeA->name == typeB->name
        && ((typeA->name == NULL || typeB->name == NULL)
            || strcmp(typeA->name, typeB->name) == 0
        )
        && orbit_typeEqual(typeA->returnType, typeB->returnType)
        && orbit_typeEqual(typeA->sibling, typeB->sibling)
        && orbit_typeEqual(typeA->children, typeB->children);
}

static int64_t typeIndex(OCType* type) {
    for(int64_t i = 0; i < _typeCount; ++i) {
        if(orbit_typeEqual(type, &_types[i])) { return i; }
    }
    return -1;
}

OCType* _install(OCType* type) {
    if(type == NULL) { return NULL; }
    int64_t idx = typeIndex(type);
    if(idx < 0) {
        _ensureTypeTable();
        _types[_typeCount] = *type;
        return &_types[_typeCount++];
    }
    else {
        return &_types[idx];
    }
}

OCType* orbit_typeInstall(OCType* type) {
    if(type == NULL) { return NULL; }
    type->returnType = orbit_typeInstall(type->returnType);
    type->sibling = orbit_typeInstall(type->sibling);
    type->children = orbit_typeInstall(type->children);
    type = _install(type);
    return type;
}