//===--------------------------------------------------------------------------------------------===
// orbit/parser/parser.h
// This source is part of Orbit - Parser
//
// Created on 2017-05-21 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent <amy@amyparent.com>
// Available under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_parser_h
#define orbit_parser_h

#include <stdbool.h>
#include <stdint.h>
#include <orbit/ast/ast.h>
#include <orbit/ast/context.h>
#include <orbit/csupport/source.h>

void orbitDumpTokens(OrbitASTContext* context);
bool orbitParse(OrbitASTContext* context);

#endif /* orbitParser_h */
