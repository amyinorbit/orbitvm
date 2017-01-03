//
//  orbit_value.c
//  OrbitVM
//
//  Created by Cesar Parent on 26/12/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include <string.h>
#include "orbit_value.h"
#include "orbit_utils.h"


void orbit_objectInit(GCObject* object, GCClass* class) {
    OASSERT(object != NULL, "Null instance error");
    
    object->class = class;
    object->isDark = false;
}

GCString* orbit_gcStringNew(OrbitVM* vm, const char* string) {
    OASSERT(string != NULL, "Null instance error");
    
    size_t length = strlen(string);
    
    GCString* object = ALLOC_FLEX(GCString, char, length+1);
    orbit_objectInit(&object->base, NULL);
    
    object->length = length;
    object->hash = orbit_hashString(string, length);
    strncpy(object->data, string, length);
    return object;
}
