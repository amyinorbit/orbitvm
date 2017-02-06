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


#endif /* orbit_pack_h */
