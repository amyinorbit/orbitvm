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

#define REGISTER_FN(name, arity) orbit_registerFn(vm, #name, &(name), (arity));

//
// Standard Library Print Functions
//

bool print_String_Void(OrbitVM* vm, GCValue* args) {
    printf("%.*s\n", (int)AS_STRING(args[0])->length, AS_STRING(args[0])->data);
    return false;
}

bool print_Number_Void(OrbitVM* vm, GCValue* args) {
    printf("%f\n", AS_NUM(args[0]));
    return false;
}

bool print_Bool_Void(OrbitVM* vm, GCValue* args) {
    printf("%s\n", IS_TRUE(args[0]) ? "true" : "false");
    return false;
}

// String Library

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
    
    REGISTER_FN(print_String_Void, 1);
    REGISTER_FN(print_Number_Void, 1);
    REGISTER_FN(print_Bool_Void, 1);
    REGISTER_FN(plus_String_String_String, 2)
}
