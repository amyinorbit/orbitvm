//
//  compiler_utils.h
//  OrbitVM
//
//  Created by Amy Parent on 2017-05-21.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef compiler_utils_h
#define compiler_utils_h

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "orbit_tokens.h"

void orbit_printSquigglies(FILE* out, uint64_t start, uint64_t length);

const char* orbit_tokenName(OCTokenType token);
const char* orbit_tokenString(OCTokenType token);

bool orbit_isBinaryOp(OCTokenType token);
bool orbit_isUnaryOp(OCTokenType token);

int orbit_binaryPrecedence(OCTokenType token);
int orbit_binaryRightAssoc(OCTokenType token);

#endif /* compiler_utils_h */
