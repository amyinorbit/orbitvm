//
//  orbit_utf8.c
//  OrbitVM
//
//  Created by Cesar Parent on 2017-02-06.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#include "orbit_utf8.h"


int8_t utf8_codepointBytes(codepoint_t point) {
    if (point < 0) return -1;
    if (point <= 0x7f) return 1;
    if (point <= 0x7ff) return 2;
    if (point <= 0xffff) return 3;
    if (point <= 0x10ffff) return 4;
    return 0;
}
