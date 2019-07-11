//===--------------------------------------------------------------------------------------------===
// orbit/utils/memory.h
// This source is part of Orbit - Utils
//
// Created on 2017-10-23 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_utils_memory_h
#define orbit_utils_memory_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdnoreturn.h>

noreturn void orbitDie(const char* message);

typedef struct _ORCObject ORCObject;
typedef void (*ORCDestructor)(void*);

/// Objects that derive from ORCObject can be managed semi-automatically using ORCRETAIN() and
/// ORCRELEASE(). When the object's [retainCount] drops to 0, its [destructor] is called and
/// and its memory freed.
struct _ORCObject {
    uint16_t        retainCount;
    ORCDestructor   destructor;
};

#define ORCRETAIN(ref) ((__typeof__(ref))orbitRcRetain((ORCObject*)(ref)))
#define ORCRELEASE(ref) (orbitRcRelease((ORCObject*)(ref), sizeof(__typeof__(*ref))))
#define ORCINIT(ref, destructor) ((__typeof__(ref))orbitRcInit((ORCObject*)(ref), (destructor)))

#define ORBIT_ALLOC(T) \
    orbitAllocator(NULL, 0, sizeof(T))
        
#define ORBIT_ALLOC_ARRAY(T, count) \
    orbitAllocator(NULL, 0, sizeof(T) * (count))

#define ORBIT_ALLOC_FLEX(T, U, count) \
    orbitAllocator(NULL, 0, sizeof(T) + (sizeof(U) * (count)))
        
#define ORBIT_REALLOC_FLEX(ptr, T, U, old, count) \
    orbitAllocator(ptr, sizeof(T) + sizeof(U) * (old), sizeof(T) + sizeof(U) * (count))
        
#define ORBIT_REALLOC_ARRAY(array, T, old, count) \
    orbitAllocator(array, sizeof(T) * (old), sizeof(T) * (count))
        
#define ORBIT_DEALLOC(ptr, T) \
    orbitAllocator(ptr, sizeof(T), 0)

#define ORBIT_DEALLOC_NOSIZE(ptr) \
    orbitAllocator(ptr, 0, 0)

#define ORBIT_DEALLOC_FLEX(ptr, T, U, count) \
    orbitAllocator(ptr, sizeof(T) + (sizeof(U) * count), 0)
        
#define ORBIT_DEALLOC_ARRAY(ptr, T, count) \
    orbitAllocator(ptr, sizeof(T) * count, 0)

#define ORBIT_GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

void* orbitAllocator(void* ptr, size_t oldSize, size_t newSize);

/// Initialises the object at [ref] with a [retainCount] of 0 and [destructor]. [ref] must
/// point to a valid ORCObject. [ref] is returned for convenience.
static inline ORCObject* orbitRcInit(ORCObject* ref, ORCDestructor destructor) {
    if(ref == NULL) { return NULL; }
    ref->retainCount = 0;
    ref->destructor = destructor;
    return ref;
}

/// Adds one to the [ref]'s retain count, 'owning' the object pointed to. [ref] must point to a
/// valid ORCObject.
static inline void* orbitRcRetain(ORCObject* ref) {
    if(ref == NULL) { return NULL; }
    ref->retainCount += 1;
    return ref;
}

/// Give up ownership of the object pointed to by [ref] by dropping its retain count by 1. If
/// the object is not owned by anything anymore, it is immediately deallocated.
static inline void orbitRcRelease(ORCObject* ref, size_t size) {
    if(ref == NULL) { return; }
    if(ref->retainCount && --ref->retainCount) { return; }
    if(ref->destructor) { ref->destructor(ref); }
    orbitAllocator(ref, size, 0);
}

#endif /* orbit_utils_memory_h */
