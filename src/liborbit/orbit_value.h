//
//  OrbitValue.h
//  OrbitVM
//
//  Created by Cesar Parent on 13/11/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//

#ifndef OrbitValue_h
#define OrbitValue_h

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "orbit_platforms.h"

typedef struct _OrbitValue      OrbitValue;
typedef struct _OrbitString     OrbitString;
typedef struct _OrbitClass      OrbitClass;
typedef struct _OrbitObject     OrbitObject;
typedef struct _OrbitInstance   OrbitInstance;
typedef struct _OrbitArray      OrbitArray;
typedef struct _OrbitDict       OrbitDict;
typedef struct _OrbitFunction   OrbitFunction;


#ifndef ORBIT_PACK_NAN

typedef enum _OrbitValueType {
    TYPE_NUMBER,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_OBJECT,
    TYPE_NULL,
    TYPE_ARRAY,
    TYPE_DICT,
} OrbitType;


typedef union _OrbitValueData {
    double          doubleValue;
    int             intValue;
    OrbitString*    stringValue;
    OrbitObject*    objectValue;
    OrbitArray*     arrayValue;
    OrbitDict*      dictValue;
    uint64_t        doubleBytes;
    uint32_t        singleBytes[2];
} OrbitValueData;

struct _OrbitValue {
    OrbitType       type;
    OrbitValueData  as;
};

#endif

#define IS_NUM(val)     ((val).type == TYPE_NUMBER)
#define IS_INT(val)     ((val).type == TYPE_INT)
#define IS_BOOL(val)    ((val).type == TYPE_BOOL)
#define IS_STRING(val)  ((val).type == TYPE_STRING)
#define IS_OBJECT(val)  ((val).type == TYPE_OBJECT)
#define IS_NULL(val)    ((val).type == TYPE_NULL)
#define IS_ARRAY(val)   ((val).type == TYPE_ARRAY)
#define IS_DICT(val)    ((val).type == TYPE_DICT)

#define AS_NUM(val)     ((val).as.doubleValue)
#define AS_BOOL(val)    ((val).as.boolValue)
#define AS_INT(val)     ((val).as.intValue)
#define AS_STRING(val)  ((val).as.stringValue)
#define AS_OBJECT(val)  ((val).as.objectValue)
#define AS_ARRAY(val)   ((val).as.arrayValue)
#define AS_DICT(val)    ((val).as.dictValue)

#define OBJPTR(val)     ((OrbitObject*)(val))
#define INSPTR(val)     ((OrbitInstance*)(val))


/**
 * A structure that describes a class/struct's layout.
 * @note    While orbit so far has no planned support for inheritance,
 *          `superClass` will be used should support be added later on. In
 *          the first spec, it should always point to NULL
 */
struct _OrbitClass {
    /// The class's name.
    const char*     name;
    /// The class's parent class.
    OrbitClass*     superClass; // Future support for inheritance?
    /// The number of fields.
    uint64_t        fieldCount;
    
};

/**
 * A heap-allocated, reference-counted Orbit VM object.
 */
struct _OrbitObject {
    /// Pointer to the object's class.
    const OrbitClass*   isa;
    /// Whether the object can be reached by the GC.
    bool                reacheable;
    /// The number of references pointing to the object.
    OrbitObject*        next;
};

struct _OrbitInstance {
    /// The object's data
    OrbitObject         object;
    /// The instance's fields
    OrbitValue          fields[ORBIT_FLEXIBLE_ARRAY_MEMB];
};

/**
 * Orbit VM's string implementation. Strings are immutable.
 */
struct _OrbitString {
    OrbitObject         object;
    /// The string's length.
    size_t              length;
    /// The string's characters.
    char                data[ORBIT_FLEXIBLE_ARRAY_MEMB];
};

/**
 * Orbit VM's array type.
 */
struct _OrbitArray {
    OrbitObject         object;
    /// The number of elements in the array.
    size_t              count;
    /// The maximum number of elements in the array.
    size_t              capacity;
    /// The array's elements.
    OrbitValue          fields[ORBIT_FLEXIBLE_ARRAY_MEMB];
};

/**
 * Orbit VM's Map/Dictionary type.
 */
struct _OrbitDict {
    OrbitObject         object;
    /// Number of items in the array
    size_t              count;
    /// Hash table buckets
    OrbitValue*         buckets[ORBIT_FLEXIBLE_ARRAY_MEMB];
};




// MARK: - Functions Definition

/**
 * Returns a hash value for an Orbit value (pointer hash for objects)
 */
static inline uint32_t orbit_valueHash(OrbitValueData value) {
    uint32_t hash = value.singleBytes[0] ^ value.singleBytes[1];
    hash ^= (hash >> 20) ^ (hash >> 12);
    hash ^= (hash >> 7) ^ (hash >> 4);
    return hash;
}

/**
 *
 */
extern OrbitObject* orbit_objectCreate(const OrbitClass* class);

/**
 *
 */
extern OrbitString* orbit_stringCreate(const OrbitClass* class, const char* contents);


#endif /* OrbitValue_h */
