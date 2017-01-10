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

// Initialises [object] as an instance of [class]. [class] can be NULL if the
// object being initialized is a class itself.
static void orbit_objectInit(OrbitVM* vm, GCObject* object, GCClass* class) {
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(object != NULL, "Null instance error");
    
    object->class = class;
    object->mark = false;
    object->next = vm->gcHead;
    vm->gcHead = object;
}

GCString* orbit_gcStringNew(OrbitVM* vm, const char* string) {
    OASSERT(vm != NULL, "Null instance error");
    
    size_t length = strlen(string);
    GCString* object = ALLOC_FLEX(vm, GCString, char, length+1);
    orbit_objectInit(vm, &object->base, NULL);
    
    object->base.type = OBJ_STRING;
    object->length = length;
    object->hash = orbit_hashString(string, length);
    memcpy(object->data, string, length);
    object->data[length] = '\0';
    return object;
}

GCInstance* orbit_gcInstanceNew(OrbitVM* vm, GCClass* class) {
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(class != NULL, "Null class error");
    
    GCInstance* object = ALLOC_FLEX(vm, GCInstance, GCValue, class->fieldCount);
    orbit_objectInit(vm, &object->base, class);
    object->base.type = OBJ_INSTANCE;
    return object;
}

GCClass* orbit_gcClassNew(OrbitVM* vm, const char* name, uint16_t fieldCount) {
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(name != NULL, "Null name error");
    
    GCClass* class = ALLOC(vm, GCClass);
    orbit_objectInit(vm, &class->base, NULL);
    class->base.type = OBJ_CLASS;
    class->super = NULL;
    class->fieldCount = fieldCount;
    orbit_stringInit(&class->name, name);
    
    return class;
}


GCMap* orbit_gcMapNew(OrbitVM* vm) {
    OASSERT(vm != NULL, "Null instance error");
    
    GCMap* map = ALLOC(vm, GCMap);
    orbit_objectInit(vm, (GCObject*)map, NULL/* TODO: replace with Map class*/);
    map->base.type = OBJ_MAP;
    
    map->capacity = GCMAP_DEFAULT_CAPACITY;
    map->mask = GCMAP_DEFAULT_CAPACITY - 1;
    map->size = 0;
    map->data = ALLOC_ARRAY(vm, GCMapEntry, GCMAP_DEFAULT_CAPACITY);
    
    for(uint32_t i = 0; i < GCMAP_DEFAULT_CAPACITY; ++i) {
        map->data[i].key = VAL_NIL;
        map->data[i].value = VAL_NIL;
    }
    
    return map;
}

GCArray* orbit_gcArrayNew(OrbitVM* vm) {
    OASSERT(vm != NULL, "Null instance error");
    
    GCArray* array = ALLOC(vm, GCArray);
    orbit_objectInit(vm, (GCObject*)array, NULL);
    array->base.type = OBJ_ARRAY;
    
    array->capacity = GCARRAY_DEFAULT_CAPACITY;
    array->size = 0;
    array->data = ALLOC_ARRAY(vm, GCValue, GCARRAY_DEFAULT_CAPACITY);
    
    return array;
}


VMFunction* orbit_gcFunctionNew(OrbitVM* vm, uint8_t* byteCode,
                                             uint16_t byteCodeLength,
                                             uint8_t constantCount) {
    OASSERT(vm != NULL, "Null instance error");
    
    VMFunction* function = ALLOC(vm, VMFunction);
    orbit_objectInit(vm, &function->base, NULL);
    function->base.type = OBJ_FUNCTION;
    function->type = FN_NATIVE;
    
    function->native.byteCode = ALLOC_ARRAY(vm, uint8_t, byteCodeLength);
    function->native.byteCodeLength = byteCodeLength;
    memcpy(function->native.byteCode, byteCode, byteCodeLength);
    
    function->native.constants = ALLOC_ARRAY(vm, GCValue, constantCount);
    function->native.constantCount = constantCount;
    
    return function;
}

void orbit_gcDeallocate(OrbitVM* vm, GCObject* object) {
    OASSERT(vm != NULL, "Null instance error");
    OASSERT(object != NULL, "Null instance error");
    
    switch(object->type) {
    case OBJ_CLASS:
        orbit_stringDeinit(&((GCClass*)object)->name);
        break;
        
    case OBJ_INSTANCE:
        break;
        
    case OBJ_STRING:
        break;
        
    case OBJ_MAP:
        DEALLOC(vm, ((GCMap*)object)->data);
        break;
    
    case OBJ_ARRAY:
        DEALLOC(vm, ((GCArray*)object)->data);
        break;
        
    case OBJ_FUNCTION:
        if(((VMFunction*)object)->type == FN_NATIVE) {
            DEALLOC(vm, ((VMFunction*)object)->native.byteCode);
            DEALLOC(vm, ((VMFunction*)object)->native.constants);
        }
        break;
        
    case OBJ_CONTEXT:
        // TODO: implement context deallocation.
        break;
    }
    DEALLOC(vm, object);
}
