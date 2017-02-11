//
//  orbit_stdlib.c
//  OrbitVM
//
//  Created by Cesar Parent on 2017-02-10.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#include <string.h>
#include "orbit_stdlib.h"
#include "orbit_value.h"
#include "orbit_vm.h"
#include "orbit_platforms.h"
#include "orbit_utf8.h"

#define REGISTER_FN(name, arity) orbit_registerFn(vm, #name, &(name), (arity))

bool currentPlatform_Void(OrbitVM* vm, GCValue* args) {
    GCString* platformString = orbit_gcStringNew(vm, ORBIT_PLATFORM);
    args[0] = MAKE_OBJECT(platformString);
    return true;
}

//
// Standard Library Print Functions
//

bool print_String_Void(OrbitVM* vm, GCValue* args) {
    printf("%.*s\n", (int)AS_STRING(args[0])->length, AS_STRING(args[0])->data);
    return true;
}

bool print_Number_Void(OrbitVM* vm, GCValue* args) {
    printf("%g\n", AS_NUM(args[0]));
    return true;
}

bool print_Bool_Void(OrbitVM* vm, GCValue* args) {
    printf("%s\n", IS_TRUE(args[0]) ? "true" : "false");
    return true;
}

// String Library

bool length_String_Number(OrbitVM* vm, GCValue* args) {
    args[0] = MAKE_NUM(AS_STRING(args[0])->length);
    return true;
}

bool characterCount_String_Number(OrbitVM* vm, GCValue* args) {
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

bool plus_String_String_String(OrbitVM* vm, GCValue* args) {
    GCString* a = AS_STRING(args[0]);
    GCString* b = AS_STRING(args[1]);
    GCString* result = orbit_gcStringReserve(vm, a->length + b->length);
    
    memcpy(result->data, a->data, a->length);
    memcpy(result->data + a->length, b->data, b->length);
    result->data[result->length] = '\0';
    args[0] = MAKE_OBJECT(result);
    return true;
}

// Reigsters [function] in [vm] with [arity] parameters, as [signature]
static void orbit_registerFn(OrbitVM* vm, const char* signature,
                             GCForeignFn function, uint8_t arity) {
    VMFunction* fn = orbit_gcFunctionForeignNew(vm, function, arity);
    GCString* sig = orbit_gcStringNew(vm, signature);
    orbit_gcMapAdd(vm, vm->dispatchTable, MAKE_OBJECT(sig), MAKE_OBJECT(fn));
}

void orbit_registerStandardLib(OrbitVM* vm) {
    REGISTER_FN(currentPlatform_Void, 0);
    
    REGISTER_FN(print_String_Void, 1);
    REGISTER_FN(print_Number_Void, 1);
    REGISTER_FN(print_Bool_Void, 1);
    
    REGISTER_FN(characterCount_String_Number, 1);
    REGISTER_FN(length_String_Number, 1);
    
    REGISTER_FN(plus_String_String_String, 2);
}
