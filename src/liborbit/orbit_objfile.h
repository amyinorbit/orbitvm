//
//  orbit_packfile.h
//  OrbitVM
//
//  Created by Cesar Parent on 2017-01-13.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#ifndef orbit_objfile_h
#define orbit_objfile_h

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
//      u16             constant_count
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
    TYPE_NUM,
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
    const char*     name;
    CPool           constants;
    uint8_t         parameterCount;
    uint16_t        requiredStack;
    uint16_t        byteCodeLength;
    uint8_t         byteCode[ORBIT_FLEXIBLE_ARRAY_MEMB];
} ObjectFunction;

typedef struct ObjectClass {
    const char*     name;
    uint16_t        fieldCount;
} ObjectClass;

typedef struct ObjectVariable {
    const char*     name;
} ObjectVariable;

#endif /* orbit_pack_h */
