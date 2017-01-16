//
//  orbit_packfile.h
//  OrbitVM
//
//  Created by Cesar Parent on 2017-01-13.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#ifndef orbit_objfile_h
#define orbit_objfile_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "orbit_platforms.h"

// Object Files are binary files that contain the bytecode and user type info
// compiled from an orbit source file. 
// 
// [Orbit Object File Format]:
//
// object_file {
//      c4              fingerprint     'OOFF'
//      u16             version_number  (0x0001)
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
//      u16             stack_effect
//      
//      u8              constant_count
//      const_struct[]  constants
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

typedef enum ObjectEntryType {
    TYPE_FUNCTION,
    TYPE_CLASS,
    TYPE_VARIABLE,
    TYPE_NUMBER,
    TYPE_STRING,
} ObjectEntryType;

typedef struct CPoolEntry {
    ObjectEntryType type;
    union {
        double      numValue;
        const char* stringValue;
    };
} CPoolEntry;

typedef struct CPool {
    uint8_t         size;
    CPoolEntry      entries[256];
} CPool;

typedef struct ObjectFunction {
    char*           name;
    CPool           constants;
    uint8_t         parameterCount;
    uint16_t        requiredStack;
    uint16_t        byteCodeLength;
    uint8_t         byteCode[ORBIT_FLEXIBLE_ARRAY_MEMB];
} ObjectFn;

typedef struct ObjectClass {
    uint16_t        fieldCount;
    char            name[ORBIT_FLEXIBLE_ARRAY_MEMB];
} ObjectClass;

typedef const char* ObjectVariable;

typedef struct array {
    uint16_t        size;
    uint16_t        capacity;
    void**          data;
} ObjectArray;

typedef struct ObjectFile {
    ObjectArray     variables;
    ObjectArray     classes;
    ObjectArray     functions;
} ObjectFile;

// Initialise [data]. This must be called for entries to be writable.
void orbit_objInit(ObjectFile* obj);

// Clear the buffers used by [data] and destroy stored entries.
void orbit_objDeinit(ObjectFile* obj);

ObjectFn* orbit_objFnNew(const char* name, uint16_t codeSize, uint8_t* code);

// Add function [name] to [obj] and return a pointer to it.
void orbit_objAddFn(ObjectFile* obj, ObjectFn* function);

// Add [string] to the [fn]'s constant pool and return its index.
uint8_t orbit_objFnAddCString(ObjectFn* fn, const char* string);

// Add [number] to [fn]'s constant pool and return its index.
uint8_t orbit_objFnAddCNumber(ObjectFn* fn, double number);

void orbit_objAddClass(ObjectFile* obj, const char* name, uint16_t fieldCount);

void orbit_objAddVariable(ObjectFile* obj, const char* name);

void orbit_objWriteFile(FILE* out, ObjectFile* obj);

void orbit_objDebugFile(FILE* in);

#endif /* orbit_pack_h */
