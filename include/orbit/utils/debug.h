//===--------------------------------------------------------------------------------------------===
// orbit/utils/debug.h
// This source is part of Orbit - Utils
//
// Created on 2018-04-15 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_utils_debug_h
#define orbit_utils_debug_h

#ifdef NDEBUG
#define ORBIT_DLOG(fmt, ...)
#else
#include <stdio.h>
#define ORBIT_DLOG(fmt, ...) fprintf(stderr, "[%s:%d] %s(): " fmt "\n",    \
    __FILE__, __LINE__, __func__ , ##__VA_ARGS__)
#endif

#endif /* orbit_utils_debug_h */
