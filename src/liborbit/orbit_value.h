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
typedef struct _VMValue     VMValue;
typedef struct _VMClass     VMClass;
typedef struct _VMObject    VMObject;
typedef struct _VMInstance  VMInstance;
typedef struct _VMString    VMString;

enum _ValueType {
    TYPE_NIL,
    TYPE_TRUE,
    TYPE_FALSE,
    TYPE_NUM,
    TYPE_STRING,
    TYPE_OBJECT
};

struct _VMValue {
    ValueType       type;
    union {
        double      numValue;
        VMString*   stringValue;
        VMInstance* objectValue;
    };
};

struct _VMClass {
    const char*     name;
    VMClass*        super;
    uint16_t        fieldCount;
};

struct _VMObject {
    VMClass*        class;
    bool            isDark;
    VMObject*       next;
};

struct _VMInstance {
    VMObject        base;
    VMValue         fields[ORBIT_FLEXIBLE_ARRAY_MEMB];
};

struct _VMString {
    VMObject        base;
    size_t          length;
    uint32_t        hash;
    char            data[ORBIT_FLEXIBLE_ARRAY_MEMB];
};

#define IS_BOOL(val)    ((val).type == TYPE_TRUE || (val).type == TYPE_FALSE)
#define IS_TRUE(val)    ((val).type != TYPE_FALSE)
#define IS_FALSE(val)   ((val).type == TYPE_FALSE)
#define IS_NIL(val)     ((val).type == TYPE_NIL)
#define IS_NUM(val)     ((val).type == TYPE_NUM)
#define IS_STRING(val)  ((val).type == TYPE_STRING)
#define IS_OBJECT(val)  ((val).type == TYPE_OBJECT)

#define AS_BOOL(val)    ((val).type == TYPE_TRUE)
#define AS_NUM(val)     ((double)(val).numValue)
#define AS_STRING(val)  ((VMString*)(val).stringValue)
#define AS_OBJECT(val)  ((VMObject*)(val).objectvalue)


extern uint32_t orbit_hashString(const char* string, size_t length);

extern uint32_t orbit_hashObject(VMInstance* object);

extern void orbit_objectInit(VMObject* object, VMClass* class);

extern VMString* orbit_stringNew(OrbitVM* vm, const char* string);

#endif /* orbit_value_h */
