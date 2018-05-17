//===--------------------------------------------------------------------------------------------===
// orbit/utils/memory.h
// This source is part of Orbit - Utils
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_utils_memory_h
#define orbit_utils_memory_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdnoreturn.h>

noreturn void orbit_die(const char* message);

typedef struct _ORCObject ORCObject;
typedef void (*ORCDestructor)(void*);

/// Objects that derive from ORCObject can be managed semi-automatically using ORCRETAIN() and
/// ORCRELEASE(). When the object's [retainCount] drops to 0, its [destructor] is called and
/// and its memory freed.
struct _ORCObject {
    uint16_t        retainCount;
    ORCDestructor   destructor;
};

#define ORCRETAIN(ref) ((__typeof__(ref))orbit_rcRetain((ORCObject*)(ref)))
#define ORCRELEASE(ref) (orbit_rcRelease((ORCObject*)(ref)))
#define ORCINIT(ref, destructor) ((__typeof__(ref))orbit_rcInit((ORCObject*)(ref), (destructor)))

#define ORBIT_ALLOC(type) (orbit_alloc(sizeof (type)))
#define ORBIT_ALLOC_FLEX(type, arrayType, count) (orbit_alloc(sizeof(type) + ((count) * sizeof(arrayType))))
#define ORBIT_REALLOC_FLEX(ptr, type, arrayType, count) (orbit_realloc((ptr), sizeof(type) + ((count) * sizeof(arrayType))))
#define ORBIT_ALLOC_ARRAY(type, count) (orbit_allocMulti(sizeof (type), (count)))
#define ORBIT_REALLOC_ARRAY(ptr, type, count) (orbit_realloc((ptr), sizeof (type) * (count)))

void* orbit_alloc(size_t size);
void* orbit_allocMulti(size_t size, size_t count);
void* orbit_realloc(void* memory, size_t size);
void orbit_dealloc(void* memory);

/// Initialises the object at [ref] with a [retainCount] of 0 and [destructor]. [ref] must
/// point to a valid ORCObject. [ref] is returned for convenience.
static inline ORCObject* orbit_rcInit(ORCObject* ref, ORCDestructor destructor) {
    if(ref == NULL) { return NULL; }
    ref->retainCount = 0;
    ref->destructor = destructor;
    return ref;
}

/// Adds one to the [ref]'s retain count, 'owning' the object pointed to. [ref] must point to a
/// valid ORCObject.
static inline void* orbit_rcRetain(ORCObject* ref) {
    if(ref == NULL) { return NULL; }
    ref->retainCount += 1;
    return ref;
}

/// Give up ownership of the object pointed to by [ref] by dropping its retain count by 1. If
/// the object is not owned by anything anymore, it is immediately deallocated.
static inline void orbit_rcRelease(ORCObject* ref) {
    if(ref == NULL) { return; }
    ref->retainCount -= 1;
    if(ref->retainCount) { return; }
    if(ref->destructor) { ref->destructor(ref); }
    orbit_dealloc(ref);
}

#endif /* orbit_utils_memory_h */
