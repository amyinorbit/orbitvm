//
//  orbit_value.h
//  OrbitVM
//
//  Created by Cesar Parent on 26/12/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#ifndef orbit_value_h
#define orbit_value_h

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <orbit/orbit.h>
#include "orbit_platforms.h"

typedef enum _ValueType     ValueType;
typedef enum _VMFnType      VMFnType;
typedef struct _VMValue     VMValue;
typedef struct _VMClass     VMClass;
typedef struct _VMObject    VMObject;
typedef struct _VMInstance  VMInstance;
typedef struct _VMString    VMString;
typedef struct _VMFunction  VMFunction;
typedef VMValue (*VMForeignFn)(VMValue*);


// The type tag of a VMValue tagged union. NIL, True and False are singletons
// to simplify dealing with them often.
//
// All numbers are double to simplify the standard library and allow
// non-programmers to not have to worry about number types. 1.8x10^308 should
// be large enough to avoid problems in most places.
//
// A value can also hold a function reference for potential closures in the
// future.
//
// TODO: add support for future VMArray and VMMap types
enum _ValueType {
    TYPE_NIL,
    TYPE_TRUE,
    TYPE_FALSE,
    TYPE_NUM,
    TYPE_STRING,
    TYPE_OBJECT
};


// Orbit's value type, used for the VM's stack and the language's variables.
//
// TODO: add support for future VMArray and VMMap types
struct _VMValue {
    ValueType       type;
    union {
        double      numValue;
        VMString*   stringValue;
        VMInstance* objectValue;
    };
};


// Orbit's class/user type representation. Even though Orbit 1 will probably
// not support inheritance (if it even supports OOP at all), we keep some space
// for a pointer to the parent class.
struct _VMClass {
    const char*     name;
    VMClass*        super;
    uint16_t        fieldCount;
};


// The base struct for any object that must be kept track of by the VM's garbage
// collector.
struct _VMObject {
    VMClass*        class;
    bool            isDark;
    VMObject*       next;
};


// Orbit's representation of an allocated instance of a language-defined class.
// 
// Half-classes like the language's primitives string, array and map do not
// require [fields] and are implemented mostly in C.
struct _VMInstance {
    VMObject        base;
    VMValue         fields[ORBIT_FLEXIBLE_ARRAY_MEMB];
};


// Orbit's primitive String type.
//
// Strings are immutable, which allows a bunch of optimisiations like storing
// length and hash, computed only once when the string is created.
struct _VMString {
    VMObject        base;
    size_t          length;
    uint32_t        hash;
    char            data[ORBIT_FLEXIBLE_ARRAY_MEMB];
};

// The type fo a VM function.
enum _VMFnType {
    FN_BYTECODE,
    FN_FOREIGN,
};


// Orbit's Function type.
//
// Function objects can hold either bytecode for functions compiled from an
// Orbit script file, or a pointer to their native implementation for functions
// declared through the C API.
struct _VMFunction {
    const char*     signature;
    VMFnType        type;
    uint8_t         parameterCount;
    union {
        VMForeignFn foreign;
        struct {
            size_t      constantCount;
            VMValue*    constants;
            size_t      byteCodeLength;
            uint8_t*    byteCode;
        }           native;
    };
};

// Orbit's call stack frame structure.
struct _VMCallFrame {
    VMFunction*     function;
    uint8_t*        ip;
    VMValue*        stackBase;
};

// Macros used to check the type of an orbit VMValue tagged union.

#define IS_BOOL(val)    ((val).type == TYPE_TRUE || (val).type == TYPE_FALSE)
#define IS_TRUE(val)    ((val).type != TYPE_FALSE)
#define IS_FALSE(val)   ((val).type == TYPE_FALSE)
#define IS_NIL(val)     ((val).type == TYPE_NIL)
#define IS_NUM(val)     ((val).type == TYPE_NUM)
#define IS_STRING(val)  ((val).type == TYPE_STRING)
#define IS_OBJECT(val)  ((val).type == TYPE_OBJECT)

// Macros used to cast [val] to a given VM type.

#define AS_BOOL(val)    ((val).type == TYPE_TRUE)
#define AS_NUM(val)     ((double)(val).numValue)
#define AS_STRING(val)  ((VMString*)(val).stringValue)
#define AS_OBJECT(val)  ((VMObject*)(val).objectvalue)

// Computes the FNV-1a hash of [string].
// This is O(n) complexity and should be used lightly. 
extern uint32_t orbit_hashString(const char* string, size_t length);

// Computes the hash code of [number].
extern uint32_t orbit_hashNumber(double number);

// Initialises [object] as an instance of [class].
extern void orbit_objectInit(VMObject* object, VMClass* class);

// Creates a garbage collected string in [vm] from the bytes in [string].
extern VMString* orbit_stringNew(OrbitVM* vm, const char* string);

#endif /* orbit_value_h */
