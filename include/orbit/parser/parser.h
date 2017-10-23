//
//  orbit/parser/parser.h
//  Orbit - Parser
//
//  Created by Amy Parent on 2017-05-21.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_parser_h
#define orbit_parser_h

#include <stdbool.h>
#include <stdint.h>
#include <orbit/ast/ast.h>
#include <orbit/source/source.h>

void orbit_dumpTokens(OCSource);

AST* orbit_parse(OCSource source);

#endif /* orbit_parser_h */
