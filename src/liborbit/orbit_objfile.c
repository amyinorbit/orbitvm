//
//  orbit_objfile.c
//  OrbitVM
//
//  Created by Cesar Parent on 2017-01-13.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#include <stdlib.h>
#include <string.h>
#include "orbit_objfile.h"
#include "orbit_vm.h"
#include "orbit_pack.h"
#include "orbit_utils.h"

// Object Files are binary files that contain the bytecode and user type info
// compiled from an orbit source file. 
// 
// [Orbit Module File Format]:
//
// object_file {
//      c4              fingerprint     'OMFF'
//      u8              version_number  (0x0001)
//
//      u16             constant_count
//      const_struct[]  constants;
//
//      u16             variable_count
//      var_struct[]    variables
//
//      u16             class_count
//      class_struct[]  classes
//
//      u16             function_count
//      func_struct[]   functions
//
//      u32             file_checksum
// }
// 
// [Entry formats]
//
// var_struct {
//     u8               tag             (TYPE_VARIABLE)
//     string_struct    name
//     u16              constant_index
// }
//
// class_struct {
//     u8               tag             (TYPE_CLASS)
//     string_struct    name
//     u16              field_count
// }
//
// func_struct {
//      u8              tag             (TYPE_FUNCTION)
//      string_struct   name
//
//      u8              param_count
//      u8              local_count
//      u16             stack_effect
//
//      u16             code_length
//      b8[]            bytecode
// }
//
//
// const_struct = (num_struct || string_struct)
//
// string_struct {
//      u8              tag             (TYPE_STRING)
//      u16             length
//      b8[]            data
// }
//
// num_struct {
//      u8              tag             (TYPE_NUM)
//      b64             data            (IEEE754-encoded double precision)
// }
//
//

#define CHECK_PTR_FALSE(error) (if(*error != PACK_NOERROR) { return false; })

static bool _checkSignature(FILE* in, OrbitPackError* error) {
    static const char signature[] = "OMFF";
    char extracted[4];
    *error = orbit_unpackBytes(in, (uint8_t*)extracted, 4);
    if(*error != PACK_NOERROR) { return false; }
    
    return memcmp(signature, extracted, 4) == 0;
}

static bool _checkVersion(FILE* in, uint8_t version, OrbitPackError* error) {
    uint8_t fileVersion = orbit_unpack8(in, error);
    if(*error != PACK_NOERROR) { return false; }
    return fileVersion == version;
}

static inline bool _loadNumber(FILE* in, GCValue* value, OrbitPackError* error) {
    double number = orbit_unpackIEEE754(in, error);
    if(*error != PACK_NOERROR) { return false; }
    *value = MAKE_NUM(number);
    return true;
}

static inline bool _loadString(OrbitVM* vm, FILE* in, GCValue* value, OrbitPackError* error) {
    uint16_t length = orbit_unpack16(in, error);
    if(*error != PACK_NOERROR) { return false; }
    
    // TODO: Check that the string is valid UTF-8
    
    GCString* string = orbit_gcStringReserve(vm, length);
    *error = orbit_unpackBytes(in, (uint8_t*)string->data, length);
    if(*error != PACK_NOERROR) { return false; }
    
    orbit_gcStringComputeHash(string);
    *value = MAKE_OBJECT(string);
    
    return true;
}

static bool _loadConstant(OrbitVM* vm, FILE* in, GCValue* value, OrbitPackError* error) {
    uint8_t tag = orbit_unpack8(in, error);
    if(*error != PACK_NOERROR) { return false; }
    
    switch(tag) {
    case OMF_STRING:
        
        return _loadString(vm, in, value, error);
        break;
        
    case OMF_NUM:
        return _loadNumber(in, value, error);
        break;
        
    default:
        break;
    }
    return false;
}

static bool _loadClass(OrbitVM* vm,
                       FILE* in,
                       GCValue* className,
                       GCValue* class,
                       OrbitPackError* error)
{
    uint8_t tag = orbit_unpack8(in, error);
    if(*error != PACK_NOERROR) { return false; }
    if(tag != OMF_CLASS) { return false; }
    
    if(!_loadString(vm, in, className, error)) { return false; }
    
    uint16_t fieldCount = orbit_unpack16(in, error);
    if(*error != PACK_NOERROR) { return false; }
    
    GCClass* impl = orbit_gcClassNew(vm, AS_STRING(*className), fieldCount);
    *class = MAKE_OBJECT(impl);
    return true;
}

static bool _loadFunction(OrbitVM* vm,
                          FILE* in,
                          GCValue* signature,
                          GCValue* function,
                          OrbitPackError* error)
{
    uint8_t tag = orbit_unpack8(in, error);
    if(*error != PACK_NOERROR) { return false; }
    if(tag != OMF_FUNCTION) { return false; }
    
    if(!_loadString(vm, in, signature, error)) { return false; }
    
    uint8_t arity = orbit_unpack8(in, error);
    if(*error != PACK_NOERROR) { return false; }
    
    uint8_t localCount = orbit_unpack8(in, error);
    if(*error != PACK_NOERROR) { return false; }
    
    uint8_t stackEffect = orbit_unpack8(in, error);
    if(*error != PACK_NOERROR) { return false; }
    
    uint16_t byteCodeLength = orbit_unpack16(in, error);
    if(*error != PACK_NOERROR) { return false; }
    
    VMFunction* impl = orbit_gcFunctionNew(vm, byteCodeLength);
    
    impl->arity = arity;
    impl->localCount = localCount;
    impl->stackEffect = stackEffect;
    impl->native.byteCodeLength = byteCodeLength;
    *error = orbit_unpackBytes(in, impl->native.byteCode, byteCodeLength);
    if(*error != PACK_NOERROR) { return false; }
    
    *function = MAKE_OBJECT(impl);
    return true;
}

VMModule* orbit_unpackModule(OrbitVM* vm, FILE* in) {
    // TODO: implementation
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(in != NULL, "Null file passed");
    
    OrbitPackError error = PACK_NOERROR;
    OrbitPackError* errorp = &error;
    VMModule* module = orbit_gcModuleNew(vm);
    
    // We don't want the module to get destroyed collected if the GC kicks
    // in while we're creating it.
    orbit_gcRetain(vm, (GCObject*)module);
    
    if(!_checkSignature(in, errorp)) { goto fail; }
    if(!_checkVersion(in, 0x01, errorp)) { goto fail; }
    
    // Read the constants in
    module->constantCount = orbit_unpack16(in, errorp);
    if(error != PACK_NOERROR) { goto fail; }
    module->constants = ALLOC_ARRAY(vm, GCValue, module->constantCount);
    
    for(uint16_t i = 0; i < module->constantCount; ++i) {
        if(!_loadConstant(vm, in, &module->constants[i], errorp)) { goto fail; }
    }
    
    // Read the globals in
    module->globalCount = orbit_unpack16(in, errorp);
    if(error != PACK_NOERROR) { goto fail; }
    module->globals = ALLOC_ARRAY(vm, VMGlobal, module->globalCount);
    
    for(uint16_t i = 0; i < module->globalCount; ++i) {
        if(!_loadString(vm, in, &module->globals[i].name, errorp)) { goto fail; }
        module->globals[i].global = VAL_NIL;
    }
    
    // TODO: Read user types in
    uint16_t classCount = orbit_unpack16(in, errorp);
    if(error != PACK_NOERROR) { goto fail; }
    for(uint8_t i = 0; i < classCount; ++i) {
        GCValue name, class;
        if(!_loadClass(vm, in, &name, &class, errorp)) { goto fail; }
        orbit_gcMapAdd(vm, module->classes, name, class);
    }
    
    // TODO: Read bytecode functions in
    uint16_t functionCount = orbit_unpack16(in, errorp);
    if(error != PACK_NOERROR) { goto fail; }
    for(uint16_t i = 0; i < functionCount; ++i) {
        GCValue signature, function;
        if(!_loadFunction(vm, in, &signature, &function, errorp)) { goto fail; }
        orbit_gcMapAdd(vm, module->dispatchTable, signature, function);
    }
    
    orbit_gcRelease(vm);
    return module;
    
fail:
    // TODO: design error model for VM
    fprintf(stderr, "error parsing module");
    return NULL;
}
