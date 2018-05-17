//===--------------------------------------------------------------------------------------------===
// orbit/ast/diag_private.h - Private structure of diagnostics objects
// This source is part of Orbit - AST
//
// Created on 2018-05-17 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2018 Amy Parent <amy@amyparent.com>
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_ast_diag_private_h
#define orbit_ast_diag_private_h
#include <orbit/ast/diag.h>

struct _OrbitDiag {
    OrbitDiagLevel  level;
    
    bool            hasSourceLoc;
    OCSourceLoc     sourceLoc;
    
    bool            hasSourceRange;
    OCSourceRange   sourceRange;
    
    const char*     format;
    
    uint32_t        paramCount;
    OrbitDiagArg    params[10];
};

#endif /* orbit_ast_diag_private_h */
