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

#define ALLOC(size) orbit_alloc(size)
#define ALLOC_FLEX(size, arr_size, count) orbit_allocFlex((size), (arr_size), (count))
#define DEALLOC(ptr) orbit_dealloc(ptr)
#define REALLOC(ptr, size) orbit_realloc((ptr), (size))

#define OASSERT(expr, error, ...)                                   \
do {                                                                \
    if(!(expr)) {                                                   \
        fprintf(stderr, error " (" #expr ")\n", ##__VA_ARGS__);     \
        exit(EXIT_FAILURE);                                         \
    }                                                               \
} while(0)

#ifndef NDEBUG
#define DBG(fmt, ...) fprintf(stderr, "[debug]: " fmt "\n", ##__VA_ARGS__)
#else
#define DBG(str, ...)
#endif

void* orbit_alloc(size_t size);

void* orbit_allocFlex(size_t size, size_t arraySize, size_t count);

void* orbit_realloc(void* ptr, size_t newSize);

void orbit_dealloc(void* ptr);


#endif /* orbit_utils_h */
