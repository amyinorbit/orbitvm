//===--------------------------------------------------------------------------------------------===
// typecheck.c - Implementation of the main type checking functions of Orbit
// This source is part of Orbit
//
// Created on 2019-07-01 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <orbit/sema/typecheck.h>
#include <orbit/utils/memory.h>
#include <assert.h>
#include "helpers.h"




void orbit_semaCheck(OCSema* self, OrbitASTContext* context) {
    assert(self && "null semantic checker error");
    assert(context && "null syntax tree error");
    
    // We need to walk the tree and
    //  1) declare symbols as we encounter them
    //  2) infer type of symbols
    //  3) deduce type of expressions and check them
    //      3a) insert conversion nodes where needed (i2f, f2i)
    //  4) Check function calls?
    //
    //
    // As enticing as it is, not quite sure the AST walker/traversal API is the best tool for
    // that job -- might be easier to roll our own here instead.
    //
    // We also need to match function overloads and implicit type conversions (not many really,
    // only Int <-> Float).
    // Overloads can be handled through a linked list of Symbol structs, which get stored in the
    // symbol table. 
    // The conversion system probably belongs in a sibling module with sort-of instruction
    // selecting. Given that not all operations will be handled through native instructions
    // (yay operator overloading), it's probably best to put all of that behind some layer of
    // abstraction so we don't have to come in here and pull everything apart when we implement
    // that.
    
}

