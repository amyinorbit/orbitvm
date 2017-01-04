//
//  orbit_value.c
//  OrbitVM
//
//  Created by Cesar Parent on 2016-12-26.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include <string.h>
#include "orbit_value.h"
#include "orbit_utils.h"
#include "orbit_vm.h"

void orbit_objectInit(OrbitVM* vm, GCObject* object, GCClass* class) {
    
    object->class = class;
    object->mark = false;
    object->type = OBJ_INSTANCE;
    
    object->next = vm->gcHead;
    vm->gcHead = object;
}

GCString* orbit_gcStringNew(OrbitVM* vm, const char* string) {
    
    size_t length = strlen(string);
    
    GCString* object = ALLOC_FLEX(vm, GCString, char, length+1);
    orbit_objectInit(vm, &object->base, NULL);
    
    object->base.type = OBJ_STRING;
    object->length = length;
    object->hash = orbit_hashString(string, length);
    memcpy(object->data, string, length);
    return object;
}

GCInstance* orbit_gcInstanceNew(OrbitVM* vm, GCClass* class) {
    GCInstance* object = ALLOC_FLEX(vm, GCInstance, GCValue, class->fieldCount);
    orbit_objectInit(vm, &object->base, class);
    return object;
}
