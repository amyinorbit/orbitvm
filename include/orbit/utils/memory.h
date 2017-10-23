//
//  orbit/utils/memory.h
//  Orbit - Utils
//
//  Created by Amy Parent on 2017-10-23.
//  Copyright © 2016-2017 Amy Parent. All rights reserved.
//
#ifndef orbit_utils_memory_h
#define orbit_utils_memory_h

#include <stdlib.h>
#include <stdnoreturn.h>

noreturn void orbit_die(const char* message);

void* orbit_alloc(size_t size);

void* orbit_allocMulti(size_t size, size_t count);

void* orbit_realloc(void* memory, size_t size);

void orbit_dealloc(void* memory);

#endif /* orbit_utils_memory_h */
