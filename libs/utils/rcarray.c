//
//  orbit/utils/rcarray.c
//  Orbit - Utils
//
//  Created by Amy Parent on 2017-11-21.
//  Copyright © 2016-2017 Amy Parent. All rights reserved.
//
#include <string.h>
#include <orbit/utils/assert.h>
#include <orbit/utils/rcarray.h>

void orbit_rcArrayDestructor(void* ref) {
    ORCArray* array = (ORCArray*)ref;
    for(uint64_t i = 0; i < array->size; ++i) {
        ORCRELEASE(array->data[i]);
    }
}

ORCArray* orbit_rcArrayNew(uint64_t capacity) {
    ORCArray* array = orbit_alloc(sizeof(ORCArray) + capacity);
    ORCINIT(array, &orbit_rcArrayDestructor);
    array->size = 0;
    array->capacity = capacity;
    return array;
}

static ORCArray* _arrayReserve(ORCArray* array, uint64_t newSize) {
    if(newSize < array->capacity) { return array; }
    while(newSize >= array->capacity) {
        array->capacity *= 2;
    }
    array = orbit_realloc(array, sizeof(ORCArray) + array->capacity);
    return array;
}

void orbit_rcArrayAppend(ORCArray** ref, void* item) {
    OASSERT(ref != NULL, "Null instance error");
    ORCArray* array = *ref;
    OASSERT(array != NULL, "Null instance error");
    
    array = _arrayReserve(array, array->size + 1);
    array->data[array->size] = ORCRETAIN(item);
    array->size += 1;
    *ref = array;
}

void orbit_rcArrayInsert(ORCArray** ref, uint64_t index, void* item) {
    OASSERT(ref != NULL, "Null instance error");
    ORCArray* array = *ref;
    OASSERT(array != NULL, "Null instance error");
    OASSERT(index <= array->size, "Index out of range");
    
    array = _arrayReserve(array, array->size + 1);
    void** insertion = &array->data[index];
    memmove(insertion+1, insertion, sizeof(void*) * (array->size - index));
    array->data[index] = ORCRETAIN(item);
    array->size += 1;
    *ref = array;
}

void orbit_rcArrayRemove(ORCArray** ref, uint64_t index) {
    OASSERT(ref != NULL, "Null instance error");
    ORCArray* array = *ref;
    OASSERT(array != NULL, "Null instance error");
    OASSERT(index < array->size, "Index out of range");

    ORCRELEASE(array->data[index]);
    
    void** deletion = &array->data[index];
    memmove(deletion, deletion+1, sizeof(void*) * (array->size - index));
    array->size -= 1;
    *ref = array;
}

void orbit_rcArrayEmpty(ORCArray** ref) {
    OASSERT(ref != NULL, "Null instance error");
    ORCArray* array = *ref;
    OASSERT(array != NULL, "Null instance error");
    for(uint64_t i = 0; i < array->size; ++i) {
        ORCRELEASE(array->data[i]);
    }
    array->size = 0;
    *ref = array;
}

