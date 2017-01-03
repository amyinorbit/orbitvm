//
//  orbit_string.c
//  OrbitVM
//
//  Created by Cesar Parent on 03/01/2017.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include "orbit_string.h"

void orbit_stringInit(String* string, const char* text) {
    OASSERT(string != NULL, "Null instance error");
    OASSERT(text != NULL, "Null text reference");
    
    string->length = strlen(text);
    string->data = ALLOC_ARRAY(char, string->length+1);
    memcpy(string->data, text, string->length+1);
    string->hash = orbit_hashString(string->data, string->length);
}

void orbit_stringDeinit(String* string) {
    OASSERT(string != NULL, "Null instance error");
    DEALLOC(string->data);
}