//===--------------------------------------------------------------------------------------------===
// buffer.c - Template-ish interface for creating dynamic runtime buffers
// This source is part of Orbit
//
// Created on 2019-07-04 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/rt2/buffer.h>
#include <orbit/rt2/memory.h>

DEFINE_BUFFER(Value, OrbitValue);
DEFINE_BUFFER(Byte, uint8_t);
DEFINE_BUFFER(Int, int32_t);
DEFINE_BUFFER(String, OrbitString*);
