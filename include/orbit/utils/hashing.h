//===--------------------------------------------------------------------------------------------===
// hashing.h - Hashing algorithms used for orbit data types.
// This source is part of Orbit
//
// Created on 2018-05-08 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_utils_hashing_h
#define orbit_utils_hashing_h
#include <stdint.h>

uint32_t orbitHashString(const char* string, uint64_t length);
uint32_t orbitHashDouble(double number);

#endif /* orbit_utils_hashing_h */
