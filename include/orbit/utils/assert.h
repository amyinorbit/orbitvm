//===--------------------------------------------------------------------------------------------===
// orbit/utils/assert.h
// This source is part of Orbit - Utils
//
// Created on 2017-06-05 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
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
