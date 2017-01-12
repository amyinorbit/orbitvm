//
//  orbit_utils.h
//  OrbitVM
//
//  Created by Cesar Parent on 2016-12-06.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#ifndef orbit_utils_h
#define orbit_utils_h

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

typedef struct _OrbitVM OrbitVM;

#define ALLOC(vm, type) \
    orbit_allocator(vm, NULL, sizeof(type))
#define ALLOC_ARRAY(vm, type, count) \
    orbit_allocator(vm, NULL, sizeof(type) * (count))
#define ALLOC_FLEX(vm, type, arrayType, count) \
    orbit_allocator(vm, NULL, sizeof(type) + (sizeof(arrayType) * (count)))
#define REALLOC_ARRAY(vm, array, type, count) \
    orbit_allocator(vm, array, (sizeof(type) * (count)))
#define DEALLOC(vm, ptr) \
    orbit_allocator(vm, ptr, 0)

// Single function used for memory allocation and deallocation in orbit.
void* orbit_allocator(OrbitVM* vm, void* ptr, size_t newSize);

// Hash functions
    
// Computes the FNV-1a hash of [string].
// This is O(n) complexity and should be used lightly. 
uint32_t orbit_hashString(const char* string, size_t length);

// Computes the hash code of [number].
uint32_t orbit_hashDouble(double number);
        
        
// Debugging facilities. When compiling a release build, OASSERT is a no-op to
// speed up the interpreter.

#ifdef NDEBUG
#define OASSERT(expr, message)
#else
#define OASSERT(expr, message)                                      \
do {                                                                \
    if(!(expr)) {                                                   \
        fprintf(stderr, "[%s:%d] Assert failed in %s(): %s\n",      \
                __FILE__, __LINE__, __func__, message);             \
        abort();                                                    \
    }                                                               \
} while(0)
#endif
    
// DBG puts out console messages, with file, line number and function name when
// running on non-release builds

#ifdef NDEBUG
#define DBG(fmt, ...)
#else
#define DBG(fmt, ...) fprintf(stderr, "[%s:%d] %s(): " fmt "\n",    \
    __FILE__, __LINE__, __func__ , ##__VA_ARGS__)
#endif

#endif /* orbit_utils_h */
