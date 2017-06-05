//
//  orbit_parser.h
//  OrbitVM
//
//  Created by Amy Parent on 2017-05-21.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_parser_h
#define orbit_parser_h

#include <stdbool.h>
#include <stdint.h>

void orbit_dumpTokens(const char* sourcePath, const char* source, uint64_t length);

bool orbit_compile(const char* sourcePath, const char* source, uint64_t length);

#endif /* orbit_parser_h */
