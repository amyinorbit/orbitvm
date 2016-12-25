//
//  orbit_utils.c
//  OrbitVM
//
//  Created by Cesar Parent on 06/12/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#include "orbit_utils.h"

void* orbit_realloc(void* ptr, size_t newSize) {
    void* mem = realloc(ptr, newSize);
    OASSERT(mem != NULL, "Error reallocating memory");
    return mem;
}
