//===--------------------------------------------------------------------------------------------===
// orbit/runtime/packfile.h
// This source is part of Orbit - Runtime
//
// Created on 2017-01-13 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_runtime_objfile_h
#define orbit_runtime_objfile_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <orbit/utils/platforms.h>
#include <orbit/runtime/value.h>

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
//      u8              tag             (TYPE_NUMBER)
//      b64             data            (IEEE754-encoded double precision)
// }
//
//

typedef enum {
    OMF_VARIABLE    = 0x01,
    OMF_CLASS       = 0x02,
    OMF_FUNCTION    = 0x03,
    OMF_STRING      = 0x04,
    OMF_NUM         = 0x05,
} OMFTag;

// Unpacks a module from [file] and adds it to [vm].
OrbitVMModule* orbit_unpackModule(OrbitVM* vm, FILE* file);


#endif /* orbit_runtime_objfile_h */
