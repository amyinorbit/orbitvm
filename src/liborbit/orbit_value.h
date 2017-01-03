//
//  orbit_value.h
//  OrbitGC
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
#include "orbit_string.h"
#include "orbit_platforms.h"

typedef enum _ValueType     ValueType;
typedef enum _GCFnType      GCFnType;
typedef struct _GCValue     GCValue;
typedef struct _GCClass     GCClass;
typedef struct _GCObject    GCObject;
typedef struct _GCInstance  GCInstance;
typedef struct _GCString    GCString;
typedef struct _VMSelector  VMSelector;
typedef struct _VMFunction  VMFunction;
typedef struct _VMCallFrame VMCallFrame;
typedef GCValue (*GCForeignFn)(GCValue*);


// The type tag of a GCValue tagged union. NIL, True and False are singletons
// to simplify dealing with them often.
//
// All numbers are double to simplify the standard library and allow
// non-programmers to not have to worry about number types. 1.8x10^308 should
// be large enough to avoid problems in most places.
//
// A value can also hold a function reference for potential closures in the
// future.
//
// TODO: add support for future GCArray and GCMap types
enum _ValueType {
    TYPE_NIL,
    TYPE_TRUE,
    TYPE_FALSE,
    TYPE_NUM,
    TYPE_STRING,
    TYPE_OBJECT
};


// Orbit's value type, used for the GC's stack and the language's variables.
//
// TODO: add support for future GCArray and GCMap types
struct _GCValue {
    ValueType       type;
    union {
        double      numValue;
        GCString*   stringValue;
        GCInstance* objectValue;
    };
};


// Orbit's class/user type representation. Even though Orbit 1 will probably
// not support inheritance (if it even supports OOP at all), we keep some space
// for a pointer to the parent class.
struct _GCClass {
    const char*     name;
    GCClass*        super;
    uint16_t        fieldCount;
};


// The base struct for any object that must be kept track of by the GC's garbage
// collector.
struct _GCObject {
    GCClass*        class;
    bool            isDark;
    GCObject*       next;
};


// Orbit's representation of an allocated instance of a language-defined class.
// 
// Half-classes like the language's primitives string, array and map do not
// require [fields] and are implemented mostly in C.
struct _GCInstance {
    GCObject        base;
    GCValue         fields[ORBIT_FLEXIBLE_ARRAY_MEMB];
};


// Orbit's primitive String type.
//
// Strings are immutable, which allows a bunch of optimisiations like storing
// length and hash, computed only once when the string is created.
struct _GCString {
    GCObject        base;
    uint64_t        length;
    uint32_t        hash;
    char            data[ORBIT_FLEXIBLE_ARRAY_MEMB];
};

// The type fo a GC function.
enum _GCFnType {
    FN_BYTECODE,
    FN_FOREIGN,
};

// Orbit's Function type.
//
// Function objects can hold either bytecode for functions compiled from an
// Orbit script file, or a pointer to their native implementation for functions
// declared through the C API.
struct _VMFunction {
    String          selector;
    GCFnType        type;
    uint8_t         parameterCount;
    union {
        GCForeignFn foreign;
        struct {
            uint8_t     constantCount;
            uint8_t     byteCodeLength;
            GCValue*    constants;
            uint8_t*    byteCode;
        } native;
    };
};

// Orbit's call stack frame structure.
struct _VMCallFrame {
    VMFunction*     function;
    uint8_t*        ip;
    GCValue*        stackBase;
};

// Macros used to check the type of an orbit GCValue tagged union.

#define IS_BOOL(val)    ((val).type == TYPE_TRUE || (val).type == TYPE_FALSE)
#define IS_TRUE(val)    ((val).type != TYPE_FALSE)
#define IS_FALSE(val)   ((val).type == TYPE_FALSE)
#define IS_NIL(val)     ((val).type == TYPE_NIL)
#define IS_NUM(val)     ((val).type == TYPE_NUM)
#define IS_STRING(val)  ((val).type == TYPE_STRING)
#define IS_OBJECT(val)  ((val).type == TYPE_OBJECT)

// Macros used to cast [val] to a given GC type.

#define AS_BOOL(val)    ((val).type == TYPE_TRUE)
#define AS_NUM(val)     ((double)(val).numValue)
#define AS_STRING(val)  ((GCString*)(val).stringValue)
#define AS_OBJECT(val)  ((GCObject*)(val).objectvalue)

// Initialises [object] as an instance of [class].
void orbit_objectInit(GCObject* object, GCClass* class);

// Creates a garbage collected string in [vm] from the bytes in [string].
GCString* orbit_gcStringNew(OrbitVM* vm, const char* string);

#endif /* orbit_value_h */
