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


void orbit_objectInit(VMObject* object, VMClass* class) {
    OASSERT(object != NULL, "Null instance error");
    
    object->class = class;
    object->isDark = false;
}

VMString* orbit_vmStringNew(OrbitVM* vm, const char* string) {
    OASSERT(string != NULL, "Null instance error");
    
    size_t length = strlen(string);
    
    VMString* object = ALLOC_FLEX(VMString, char, length+1);
    orbit_objectInit(&object->base, NULL);
    
    object->length = length;
    object->hash = orbit_hashString(string, length);
    strncpy(object->data, string, length);
    return object;
}
