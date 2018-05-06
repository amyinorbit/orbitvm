//===--------------------------------------------------------------------------------------------===
// orbit/stdlib/stdlib.c
// This source is part of Orbit - Standard Library
//
// Created on 2017-02-10 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <string.h>
#include <math.h>
#include <orbit/runtime/value.h>
#include <orbit/runtime/vm.h>
#include <orbit/utils/platforms.h>
#include <orbit/utils/utf8.h>
#include <orbit/stdlib/stdlib.h>

#define REGISTER_FN(name, arity) orbit_registerFn(vm, #name, &(name), (arity))

bool currentPlatform(OrbitVM* vm, OrbitValue* args) {
    OrbitGCString* platformString = orbit_gcStringNew(vm, ORBIT_PLATFORM);
    args[0] = MAKE_OBJECT(platformString);
    return true;
}

//
// Basic maths functions
//

bool sqrt_Num(OrbitVM* vm, OrbitValue* args) {
    args[0] = MAKE_NUM(sqrt(AS_NUM(args[0])));
    return true;
}

//
// Standard Library Print Functions
//

bool print_String(OrbitVM* vm, OrbitValue* args) {
    printf("%.*s\n", (int)AS_STRING(args[0])->length, AS_STRING(args[0])->data);
    return false;
}

bool print_Number(OrbitVM* vm, OrbitValue* args) {
    printf("%.9g\n", AS_NUM(args[0]));
    return false;
}

bool print_Bool(OrbitVM* vm, OrbitValue* args) {
    printf("%s\n", IS_TRUE(args[0]) ? "true" : "false");
    return false;
}

// String Library

bool length_String(OrbitVM* vm, OrbitValue* args) {
    args[0] = MAKE_NUM(AS_STRING(args[0])->length);
    return true;
}

bool characterCount_String(OrbitVM* vm, OrbitValue* args) {
    uint64_t index = 0, count = 0, length = AS_STRING(args[0])->length;
    char* characters = AS_STRING(args[0])->data;
    
    while(index < length) {
        codepoint_t c = utf8_getCodepoint(characters+index, length-index);
        int clen = utf8_codepointSize(c);
        index += clen;
        count += 1;
    }
    
    args[0] = MAKE_NUM(count);
    return true;
}

bool plus_String_String(OrbitVM* vm, OrbitValue* args) {
    OrbitGCString* a = AS_STRING(args[0]);
    OrbitGCString* b = AS_STRING(args[1]);
    OrbitGCString* result = orbit_gcStringReserve(vm, a->length + b->length);
    
    memcpy(result->data, a->data, a->length);
    memcpy(result->data + a->length, b->data, b->length);
    result->data[result->length] = '\0';
    args[0] = MAKE_OBJECT(result);
    return true;
}

// Reigsters [function] in [vm] with [arity] parameters, as [signature]
static void _registerFn(OrbitVM* vm, const char* signature,
                        GCForeignFn function, uint8_t arity) {
    VMFunction* fn = orbit_gcFunctionForeignNew(vm, function, arity);
    OrbitGCString* sig = orbit_gcStringNew(vm, signature);
    orbit_gcMapAdd(vm, vm->dispatchTable, MAKE_OBJECT(sig), MAKE_OBJECT(fn));
}

void orbit_registerStandardLib(OrbitVM* vm) {
    
    _registerFn(vm, "currentPlatform()", currentPlatform, 0);
    _registerFn(vm, "sqrt(Num)", sqrt_Num, 1);
    
    _registerFn(vm, "print(String)", print_String, 1);
    _registerFn(vm, "print(Num)", print_Number, 1);
    _registerFn(vm, "print(Bool)", print_Bool, 1);

    _registerFn(vm, "length(String)", length_String, 1);
    _registerFn(vm, "characterCount(String)", characterCount_String, 1);
    _registerFn(vm, "+(String,String)", plus_String_String, 2);
}
