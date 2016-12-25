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

#define ALLOC(vm, type) \
    (type*)orbit_realloc(NULL, sizeof(type))

#define ALLOC_ARRAY(vm, type, count) \
    (type*)orbit_realloc(NULL, sizeof(type) * (count))

#define ALLOC_FLEX(vm, type, arrayType, count) \
    (type*)orbit_realloc(NULL, syzeof(type) + (sizeof(arrayType) * (count)))

#define DEALLOC(vm, ptr) \
    orbit_realloc(ptr, 0)

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
#define DBG(fmt, ...)
#else
#define DBG(fmt, ...) fprintf(stderr, "[%s:%d] %s(): " fmt "\n",    \
    __FILE__, __LINE__, __func__ , ##__VA_ARGS__)
#endif

void* orbit_realloc(void* ptr, size_t newSize);


#endif /* orbit_utils_h */
