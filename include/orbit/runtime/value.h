//===--------------------------------------------------------------------------------------------===
// orbit/runtime/value.h
// This source is part of OrbitGC
//
// Created on 2016-12-26 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_value_h
#define orbit_value_h

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <orbit/orbit.h>
#include <orbit/utils/platforms.h>

typedef enum _OrbitValueKind    OrbitValueKind;
typedef enum _OrbitFnKind       OrbitFnKind;
typedef enum _OrbitObjKind      OrbitObjKind;
typedef struct _OrbitValue      OrbitValue;
typedef struct _OrbitGCClass    OrbitGCClass;
typedef struct _OrbitGCObject   OrbitGCObject;
typedef struct _OrbitGCInstance OrbitGCInstance;
typedef struct _OrbitGCString   OrbitGCString;
typedef struct _OrbitGCMap      OrbitGCMap;
typedef struct _OrbitGCArray    OrbitGCArray;
typedef struct _VMFunction      VMFunction;
typedef struct _VMCallFrame     VMCallFrame;
typedef struct _VMGlobal        VMGlobal;
typedef struct _VMModule        VMModule;
typedef struct _VMTask          VMTask;
typedef bool (*GCForeignFn)(OrbitVM* vm, OrbitValue*);


// The type tag of a OrbitValue tagged union. NIL, True and False are singletons
// to simplify dealing with them often.
//
// All numbers are double to simplify the standard library and allow
// non-programmers to not have to worry about number types. 1.8x10^308 should
// be large enough to avoid problems in most places.
//
// A value can also hold a function reference for potential closures in the
// future.
enum _OrbitValueKind {
    ORBIT_VK_NIL,
    ORBIT_VK_TRUE,
    ORBIT_VK_FALSE,
    ORBIT_VK_NUM,
    ORBIT_VK_OBJECT
};


// Orbit's value type, used for the GC's stack and the language's variables.
struct _OrbitValue {
    OrbitValueKind  type;
    union {
        double      numValue;
        void*       objectValue;
    };
};

// The type of a garbage-collected object. This is used to decide how to collect
// the object, and wether it has fields pointing to other objects in the graph.
enum _OrbitObjKind {
    ORBIT_OBJK_CLASS,
    ORBIT_OBJK_INSTANCE,
    ORBIT_OBJK_STRING,
    ORBIT_OBJK_MAP,
    ORBIT_OBJK_ARRAY,
    ORBIT_OBJK_FUNCTION,
    ORBIT_OBJK_MODULE,
    ORBIT_OBJK_TASK,
};


// The base struct for any object that must be kept track of by the GC's garbage
// collector.
struct _OrbitGCObject {
    OrbitGCClass*   class;
    OrbitObjKind    type;
    bool            mark;
    OrbitGCObject*  next;
};


// Orbit's class/user type representation. Even though Orbit 1 will probably
// not support inheritance (if it even supports OOP at all), we keep some space
// for a pointer to the parent class.
struct _OrbitGCClass {
    OrbitGCObject   base;
    OrbitGCString*  name;
    OrbitGCClass*   super;
    uint16_t        fieldCount;
    OrbitGCMap*     methods;
};


// Orbit's representation of an allocated instance of a language-defined class.
// 
// Half-classes like the language's primitives string, array and map do not
// require [fields] and are implemented mostly in C.
struct _OrbitGCInstance {
    OrbitGCObject   base;
    OrbitValue      fields[ORBIT_FLEXIBLE_ARRAY_MEMB];
};


// Orbit's primitive String type.
//
// Strings are immutable, which allows a bunch of optimisiations like storing
// length and hash, computed only once when the string is created.
struct _OrbitGCString {
    OrbitGCObject   base;
    uint64_t        length;
    uint32_t        hash;
    char            data[ORBIT_FLEXIBLE_ARRAY_MEMB];
};

// Orbit's primitive map's entry type. Key can be any primitive value (string
// or number).
typedef struct {
    OrbitValue      key;
    OrbitValue      value;
} OrbitGCMapEntry;

// The default capacity of a hash map. Must be a power of two to allow for
// AND modulo hack.
#define GCMAP_DEFAULT_CAPACITY 32

// Orbit's associative array type, implemented as an open-addressed, linear
// probed hash map.
struct _OrbitGCMap {
    OrbitGCObject       base;
    uint64_t            mask;
    uint64_t            size;
    uint64_t            capacity;
    OrbitGCMapEntry*    data;
};

#define GCARRAY_DEFAULT_CAPACITY 32

// Orbit's dynamic array type.
struct _OrbitGCArray {
    OrbitGCObject   base;
    uint64_t        size;
    uint64_t        capacity;
    OrbitValue*     data;
};

// The type fo a GC function.
enum _OrbitFnKind {
    ORBIT_FK_NATIVE,
    ORBIT_FK_FOREIGN,
};

// Orbit's native function type, used for bytecode-compiled functions.
typedef struct _GCNativeFn {
    uint16_t        byteCodeLength;
    uint8_t*        byteCode;
} GCNativeFn;

// Orbit's Function type.
//
// Function objects can hold either bytecode for functions compiled from an
// Orbit script file, or a pointer to their native implementation for functions
// declared through the C API.
struct _VMFunction {
    OrbitGCObject   base;
    OrbitFnKind     type;
    VMModule*       module;
    uint8_t         arity;
    uint8_t         localCount;
    uint8_t         stackEffect;
    union {
        GCForeignFn foreign;
        GCNativeFn  native;
    };
};

// Orbit's call stack frame structure.
struct _VMCallFrame {
    VMTask*         task;
    VMFunction*     function;
    uint8_t*        ip;
    OrbitValue*     stackBase;
};

// Tasks hold the data required to execute bytecode: an operand stack for
// temporary results, as well as a call stack for function invocation and
// return.
struct _VMTask {
    OrbitGCObject   base;
    
    uint64_t        stackCapacity;
    OrbitValue*     sp;
    OrbitValue*     stack;
    
    uint64_t        frameCount;
    uint64_t        frameCapacity;
    VMCallFrame*    frames;
};

struct _VMGlobal {
    OrbitValue      name;
    OrbitValue      global;
};

// VMModule holds all that is needed for a bytecode file to be executed.
// A module is created when a bytecode file is loaded into the VM, and can be
// used to hold state in between C API function calls.
struct _VMModule {
    OrbitGCObject   base;
    
    uint16_t        constantCount;
    OrbitValue*     constants;
    
    uint16_t        globalCount;
    VMGlobal*       globals;
};

// Macros used to check the type of an orbit OrbitValue tagged union.

#define MAKE_NUM(num)   ((OrbitValue){ORBIT_VK_NUM, {.numValue=(num)}})
#define MAKE_BOOL(val)  ((OrbitValue){(val)? ORBIT_VK_TRUE : ORBIT_VK_FALSE, {.numValue=0}})
#define MAKE_OBJECT(obj)((OrbitValue){ORBIT_VK_OBJECT, {.objectValue=(obj)}})

#define VAL_NIL         ((OrbitValue){ORBIT_VK_NIL, {.objectValue=NULL}})
#define VAL_TRUE        ((OrbitValue){ORBIT_VK_TRUE, {.objectValue=NULL}})
#define VAL_FALSE       ((OrbitValue){ORBIT_VK_FALSE, {.objectValue=NULL}})

#define IS_BOOL(val)    ((val).type == ORBIT_VK_TRUE || (val).type == ORBIT_VK_FALSE)
#define IS_TRUE(val)    ((val).type == ORBIT_VK_TRUE || (IS_NUM(val) && AS_NUM(val) != 0.0))
#define IS_FALSE(val)   (!IS_TRUE(val))
#define IS_NIL(val)     ((val).type == ORBIT_VK_NIL)
#define IS_NUM(val)     ((val).type == ORBIT_VK_NUM)
#define IS_OBJECT(val)  ((val).type == ORBIT_VK_OBJECT)
#define IS_INSTANCE(val)(IS_OBJECT(val) && AS_OBJECT(val)->type == ORBIT_OBJK_INSTANCE)
#define IS_STRING(val)  (IS_OBJECT(val) && AS_OBJECT(val)->type == ORBIT_OBJK_STRING)
#define IS_CLASS(val)   (IS_OBJECT(val) && AS_OBJECT(val)->type == ORBIT_OBJK_CLASS)
#define IS_FUNCTION(val)(IS_OBJECT(val) && AS_OBJECT(val)->type == ORBIT_OBJK_FUNCTION)
#define IS_MODULE(val)  (IS_OBJECT(val) && AS_OBJECT(val)->type == ORBIT_OBJK_MODULE)

// Macros used to cast [val] to a given GC type.

#define AS_BOOL(val)    ((val).type == ORBIT_VK_TRUE)
#define AS_NUM(val)     ((double)(val).numValue)
#define AS_OBJECT(val)  ((OrbitGCObject*)(val).objectValue)
#define AS_CLASS(val)   ((OrbitGCClass*)AS_OBJECT(val))
#define AS_INST(val)    ((OrbitGCInstance*)AS_OBJECT(val))
#define AS_STRING(val)  ((OrbitGCString*)AS_OBJECT(val))
#define AS_FUNCTION(val)((VMFunction*)AS_OBJECT(val))

// Creates a garbage collected string in [vm] from the bytes in [string].
OrbitGCString* orbit_gcStringNew(OrbitVM* vm, const char* string);

// Creates a garbage collected string in [vm] with [size] bytes.
OrbitGCString* orbit_gcStringReserve(OrbitVM* vm, size_t size);

// Recomputes the hash of [string] and stores it.
void orbit_gcStringComputeHash(OrbitGCString* string);

// Creates a garbage collected instance of [class] in [vm].
OrbitGCInstance* orbit_gcInstanceNew(OrbitVM* vm, OrbitGCClass* class);

// Creates a new class meta-object in [vm] named [className].
OrbitGCClass* orbit_gcClassNew(OrbitVM* vm, OrbitGCString* name, uint16_t fieldCount);

// Creates a new hash map object in [vm];
OrbitGCMap* orbit_gcMapNew(OrbitVM* vm);

// Add a the [key] ==> [value] pair to [map]. [map] is grown if necessary.
void orbit_gcMapAdd(OrbitVM* vm, OrbitGCMap* map, OrbitValue key, OrbitValue value);

// Fetch the value for [key] in [map] into [value]. If [key] does not exist in
// [map], returns false.
bool orbit_gcMapGet(OrbitGCMap* map, OrbitValue key, OrbitValue* value);

// Remove the value for [key] in [map] if it exists.
void orbit_gcMapRemove(OrbitVM* vm, OrbitGCMap* map, OrbitValue key);

// Creates a new array in [vm].
OrbitGCArray* orbit_gcArrayNew(OrbitVM* vm);

// Add [value] to [array].
void orbit_gcArrayAdd(OrbitVM* vm, OrbitGCArray* array, OrbitValue value);

// Fetch the value at [index] in [array] into [value]. If [index] is out of
// bounds, returns false.
bool orbit_gcArrayGet(OrbitGCArray* array, uint32_t index, OrbitValue* value);

// Remove the value at [index] in [array]. If [index] is out of bounds, returns
// false. Shrink [array] if necessary.
bool orbit_gcArrayRemove(OrbitVM* vm, OrbitGCArray* array, uint32_t index);

// Creates a native bytecode function.
VMFunction* orbit_gcFunctionNew(OrbitVM* vm, uint16_t byteCodeLength);

// Creates a new foreign function
VMFunction* orbit_gcFunctionForeignNew(OrbitVM* vm, GCForeignFn ffi, uint8_t arity);

// Creates a module that can be populated with the contents of a bytecode file.
// The resulting module has no constant or variable space reserved. This must
// be handled by the function generating the module.
VMModule* orbit_gcModuleNew(OrbitVM* vm);

// Creates a new task in [vm] and push [function] on the call stack;
VMTask* orbit_gcTaskNew(OrbitVM* vm, VMFunction* function);

// Deallocates [object].
void orbit_gcDeallocate(OrbitVM* vm, OrbitGCObject* object);

#endif /* orbit_value_h */
