//===--------------------------------------------------------------------------------------------===
// orbit/runtime/utils.h
// This source is part of Orbit - Runtime
//
// Created on 2016-12-06 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_utils_h
#define orbit_utils_h
#include <orbit/common.h>
#include <orbit/utils/memory.h>

void* orbit_gcalloc(OrbitGC* gc, void* ptr, size_t oldSize, size_t newSize);

#endif /* orbit_utils_h */
