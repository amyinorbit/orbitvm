//===--------------------------------------------------------------------------------------------===
// value_string.h - Orbit Runtime's string object API
// This source is part of Orbit
//
// Created on 2019-06-28 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_value_string_h
#define orbit_value_string_h
#include <orbit/common.h>
#include <orbit/rt2/value_object.h>

OrbitString* orbit_stringNew(OrbitVM* vm, const char* data, int32_t count);

#endif