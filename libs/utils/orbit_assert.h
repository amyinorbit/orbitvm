//
//  orbit_assert.h
//  Orbit - Utils
//
//  Created by Amy Parent on 2017-06-05.
//  Copyright © 2016-2017 Amy Parent. All rights reserved.
//
#ifndef orbit_assert_h
#define orbit_assert_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
        
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

#endif /* orbit_assert_h */
