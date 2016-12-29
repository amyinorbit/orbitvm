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


typedef union {
    double      number;
    uint32_t    raw[2];
} RawDouble;

uint32_t orbit_hashString(const char* string, size_t length) {
    OASSERT(string != NULL, "Null instance error");
    
    //Fowler-Noll-Vo 1a hash
    //http://create.stephan-brumme.com/fnv-hash/
    uint32_t hash = 0x811C9DC5;
    for(size_t i = 0; i < length; ++i) {
        hash ^= string[i];
        hash *= 0x01000193;
    }
    return hash;
}

uint32_t orbit_hashNumber(double number) {
    RawDouble bits = {.number = number};
    return bits.raw[0] ^ bits.raw[1];
}

void orbit_objectInit(VMObject* object, VMClass* class) {
    OASSERT(object != NULL, "Null instance error");
    
    object->class = class;
    object->isDark = false;
}

VMString* orbit_stringNew(OrbitVM* vm, const char* string) {
    OASSERT(string != NULL, "Null instance error");
    
    size_t length = strlen(string);
    
    VMString* object = ALLOC_FLEX(_, VMString, char, length+1);
    orbit_objectInit(&object->base, NULL);
    
    object->length = length;
    object->hash = orbit_hashString(string, length);
    strncpy(object->data, string, length);
    return object;
}
