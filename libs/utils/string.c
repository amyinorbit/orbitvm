//
//  orbit/utils/string.c
//  Orbit - Utils
//
//  Created by Amy Parent on 2017-11-21.
//  Copyright © 2016-2017 Amy Parent. All rights reserved.
//
#include <string.h>
#include <orbit/utils/string.h>

UTFString* orbit_utfStringNew(const char* cString, size_t length) {
    UTFString* string = orbit_alloc(sizeof(UTFString) + length + 1);
    ORCINIT(string, NULL);
    memcpy(string->data, cString, length);
    string->data[length] = 0;
    string->capacity = length + 1;
    string->length = length;
    return string;
}

UTFString* orbit_utfStringNewWithCapacity(uint64_t capacity) {
    UTFString* string = orbit_alloc(sizeof(UTFString) + capacity);
    ORCINIT(string, NULL);
    memset(string->data, 0, capacity);
    string->capacity = capacity;
    string->length = 0;
    return string;
}

static UTFString* _stringReserve(UTFString* string, size_t newChars) {
    size_t required = string->length + newChars + 1;
    if(required < string->capacity) { return string; }
    while(required >= string->capacity) {
        string->capacity *= 2;
    }
    string = orbit_realloc(string, sizeof(UTFString) + string->capacity);
    return string;
}

UTFString* orbit_utfStringAppend(UTFString* string, codepoint_t c) {
    int8_t size = utf8_codepointSize(c);
    if(size < 0) { return string; }
    
    string = _stringReserve(string, size);
    utf8_writeCodepoint(c, &string->data[string->length],
                           string->capacity - string->length);
    string->length += size;
    string->data[string->length] = '\0';
    return string;
}

UTFString* orbit_utfStringCompact(UTFString* string) {
    size_t newCapacity = string->length + 1;
    string = orbit_realloc(string, sizeof(UTFString) + newCapacity);
    string->capacity = newCapacity;
    return string;
}
