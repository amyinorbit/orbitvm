//===--------------------------------------------------------------------------------------------===
// value.h - The value API for Orbit Runtime 2.0 (static typed)
// This source is part of Orbit
//
// Created on 2019-06-27 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_value_h
#define orbit_value_h
#include <orbit/common.h>

typedef struct sOrbitValue OrbitValue;
// typedef int32_t OrbitValue;

struct sOrbitValue {
    enum {ORBIT_VK_INT, ORBIT_VK_FLOAT} kind;
    union {
        int32_t intValue;
        float   floatValue;
    };
};

#define IS_INT(value) ((value).kind == ORBIT_VK_INT)
#define AS_INT(value) ((value).intValue)
#define MAKE_INT(num) ((OrbitValue){ .kind=ORBIT_VK_INT, .intValue=(num) })

#define IS_FLOAT(value) ((value).kind == ORBIT_VK_FLOAT)
#define AS_FLOAT(value) ((value).floatValue)
#define MAKE_FLOAT(num) ((OrbitValue){ .kind=ORBIT_VK_FLOAT, .floatValue=(num) })

#endif