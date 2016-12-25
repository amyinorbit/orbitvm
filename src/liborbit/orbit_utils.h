//
//  orbit_utils.h
//  OrbitVM
//
//  Created by Cesar Parent on 06/12/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#ifndef orbit_utils_h
#define orbit_utils_h

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define REALLOC(ptr, size) orbit_realloc((ptr), (size))

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

#ifdef NDEBUG
#define DBG(str, ...)
#else
#define DBG(fmt, ...) fprintf(stderr, "[debug]: " fmt "\n", ##__VA_ARGS__)
#endif

void* orbit_alloc(size_t size);

void* orbit_allocFlex(size_t size, size_t arraySize, size_t count);

void* orbit_realloc(void* ptr, size_t newSize);

void orbit_dealloc(void* ptr);


#endif /* orbit_utils_h */
