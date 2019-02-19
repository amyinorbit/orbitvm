//===--------------------------------------------------------------------------------------------===
// orbit/utils/platforms.h
// This source is part of Orbit - Utils
//
// Created on 2016-11-14 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_platforms_h
#define orbit_platforms_h

#ifdef _WIN32
#define ORBIT_PLATFORM "Windows"
#elif __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
#define ORBIT_PLATFORM "iOS-x86"
#elif TARGET_OS_IPHONE
#define ORBIT_PLATFORM "iOS-arm"
#elif TARGET_OS_MAC
#define ORBIT_PLATFORM "macOS"
#endif
#elif __linux__
#define ORBIT_PLATFORM "Linux"
#elif __unix__
#define ORBIT_PLATFORM "UNIX"
#else
#define ORBIT_PLATFORM "Unknown Platform"
#endif

#ifndef _MSC_VER
#define ORBIT_FORBIT_AST_INTERPRET
#endif

#if __STDC_VERSION__ >= 199901L
#define ORBIT_FLEXIBLE_ARRAY_MEMB   
#else
#define ORBIT_FLEXIBLE_ARRAY_MEMB  0
#endif

#endif /* orbit_platforms_h */
