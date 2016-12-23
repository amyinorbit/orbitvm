//
//  OrbitValue.c
//  OrbitVM
//
//  Created by Cesar Parent on 13/11/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include <stdlib.h>
#include <string.h>
#include "orbit_value.h"
#include "orbit_utils.h"

#define OBJSIZE(class) (sizeof(OrbitObject)+(class->fieldCount*sizeof(OrbitValue)))

OrbitObject* orbit_objectCreate(const OrbitClass* class) {
    size_t size = OBJSIZE(class);
    OrbitObject* obj = calloc(size, 1);
    OASSERT(obj != NULL, "allocation failure");
    memset(obj, 0, size);
    obj->isa = class;
    return obj;
}

OrbitString* orbit_stringCreate(const OrbitClass* class, const char* contents) {
    size_t length = strlen(contents);
    size_t size = OBJSIZE(class) + length;
    
    OrbitString* str = calloc(size, 1);
    OASSERT(str != NULL, "allocation failure");
    memset(str, 0, size);
    OBJPTR(str)->isa = class;
    
    
    return str;
}
